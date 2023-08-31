/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/ApplicationUtil.h>
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/EngineSettingsSerialization.h>
#include <Sprocket/MainWindow.h>
#include <Sprocket/MainWindowUtil.h>
#include <Sprocket/ContextAction/CreateContextActions.h>
#include <Sprocket/Input/ViewportInputSystem.h>
#include <Sprocket/Input/InputPlatformQt.h>
#include <Sprocket/Scenario/EntityObjectType.h>
#include <Sprocket/Viewport/ScenarioObjectPicker.h>
#include <Sprocket/Scenario/ScenarioSelectionModel.h>
#include <Sprocket/Viewport/DefaultViewportMouseEventHandler.h>
#include <Sprocket/ThirdParty/DarkStyle.h>
#include <Sprocket/Widgets/ScenarioPropertyEditorWidget.h>
#include <Sprocket/Widgets/ScenarioObjectsEditorWidget.h>
#include <Sprocket/Widgets/TimelineControlWidget.h>
#include <Sprocket/Widgets/ViewportWidget.h>

#include <SkyboltCommon/ContainerUtility.h>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Stringify.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/GetExecutablePath.h>
#include <SkyboltEngine/Input/InputSystem.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltSim/World.h>

#include <QSurfaceFormat>

using namespace skybolt;

static std::vector<EditorPluginPtr> loadEditorPlugins(const std::vector<EditorPluginFactory>& editorPluginFactories, const EditorPluginConfig& config)
{
	std::vector<EditorPluginPtr> plugins;
	transform(editorPluginFactories, plugins, [config](const EditorPluginFactory& factory) {
		return factory(config);
	});
	return plugins;
}

static vis::VisRootPtr createVisRoot(const EngineRoot& engineRoot)
{
	auto visRoot = std::make_shared<vis::VisRoot>(getDisplaySettingsFromEngineSettings(engineRoot.engineSettings));
	// Since Qt manages the OpenGL context, ensure OSG renders on the Qt thread
	visRoot->getViewer().setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	return visRoot;
}

class Application : public QApplication
{
public:
	Application(const std::vector<PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& editorPluginFactories, int argc, char* argv[]) :
		QApplication(argc, argv)
	{
		QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/qtplugins");

		setStyle(new DarkStyle);

		QSettings settings(QApplication::applicationName());
		nlohmann::json engineSettings = readOrCreateEngineSettingsFile(settings);
		std::shared_ptr<EngineRoot> engineRoot = EngineRootFactory::create(enginePluginFactories, engineSettings);

		mMainWindow.reset(new MainWindow([&] {
			MainWindowConfig c;
			c.engineRoot = engineRoot;
			return c;
		}()));

		auto selectionModel = new ScenarioSelectionModel(mMainWindow.get());

		auto inputPlatform = std::make_shared<InputPlatformQt>();
		CameraInputAxes axes = createDefaultCameraInputAxes(*inputPlatform);
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, toValuesVector(axes)));

		auto viewportInputSystem = std::make_shared<ViewportInputSystem>(inputPlatform, axes);
		engineRoot->systemRegistry->push_back(viewportInputSystem);

		vis::VisRootPtr visRoot = createVisRoot(*engineRoot);
		QObject::connect(mMainWindow.get(), &MainWindow::updated, [=] {
			visRoot->render();
		});

		{
			EditorPluginConfig config;
			config.uiController = std::make_shared<UiController>();
			config.uiController->toolWindowRaiser = [this](QWidget* widget) {
				mMainWindow->raiseToolWindow(widget);
			};
			config.engineRoot = engineRoot.get();
			config.selectionModel = selectionModel;
			config.inputPlatform = inputPlatform;
			config.visRoot = visRoot.get();
			config.mainWindow = mMainWindow.get();

			mEditorPlugins = loadEditorPlugins(editorPluginFactories, config);
			addToolWindows(*mMainWindow, mEditorPlugins);
		}

		const ScenarioObjectTypeMap& scenarioObjectTypes = getSceneObjectTypes(mEditorPlugins, engineRoot.get());

		{
			auto widget = new ScenarioPropertyEditorWidget([&] {
				ScenarioPropertyEditorWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.selectionModel = selectionModel;
				c.propertyModelFactoryMap = getPropertyModelFactories(mEditorPlugins, engineRoot.get());
				c.widgetFactoryMap = getPropertyEditorWidgetFactories(mEditorPlugins);
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Properties", widget);
		}
		{
			auto widget = new ScenarioObjectsEditorWidget([&] {
				ScenarioObjectsEditorWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.selectionModel = selectionModel;
				c.scenarioObjectTypes = scenarioObjectTypes;
				c.contextActions = createContextActions(*engineRoot);
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Explorer", widget);
		}

		auto viewportMouseEventHandler = std::make_shared<DefaultViewportMouseEventHandler>([&] {
				DefaultViewportMouseEventHandlerConfig c;
				c.engineRoot = engineRoot.get();
				c.viewportInput = viewportInputSystem;
				c.scenarioSelectionModel = selectionModel;
				c.selectionPredicate = ScenarioObjectPredicateAlways;
				return c;
			}());

		{
			auto widget = new ViewportWidget([&] {
				ViewportWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.visRoot = visRoot;
				c.selectionModel = selectionModel;
				c.scenarioObjectPicker = createScenarioObjectPicker(scenarioObjectTypes);
				c.contextActions = createContextActions(*engineRoot);
				c.projectFilenameGetter = [this] { return mMainWindow->getProjectFilename().toStdString(); };
				c.parent = mMainWindow.get();
				return c;
			}());
			widget->addMouseEventHandler(viewportMouseEventHandler);

			for (const auto& layer : getEntityVisibilityLayers(mEditorPlugins))
			{
				widget->addVisibilityFilterableSubMenu(QString::fromStdString(layer.first), layer.second);
			}

			mMainWindow->addToolWindow("Viewport", widget);

			QObject::connect(mMainWindow.get(), &MainWindow::updated, widget, &ViewportWidget::update);
			connectJsonProjectSerializable(*mMainWindow, *widget);
		}
		{
			auto window = new TimelineControlWidget(&engineRoot->scenario->timeSource, mMainWindow.get());
			mMainWindow->addToolWindow("Time Control", window);
		}

		try
		{
			if (argc >= 2)
			{
				QString filename(argv[1]);
				mMainWindow->openProject(filename, MainWindow::OverwriteMode::OverwriteWithoutPrompt);
			}
			else
			{
				mMainWindow->newProject(MainWindow::OverwriteMode::OverwriteWithoutPrompt);
			}
		}
		catch (...)
		{
			mMainWindow.reset();
			throw;
		}

		const QString defaultWindowLayoutState = "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";
		mMainWindow->restoreToolWindowsState(defaultWindowLayoutState);
		mMainWindow->showMaximized();
	}

	~Application()
	{
		mMainWindow.reset();
	}

private:
	std::unique_ptr<MainWindow> mMainWindow;
	std::vector<EditorPluginPtr> mEditorPlugins;
};

int main(int argc, char *argv[])
{
	try
	{
		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
		QCoreApplication::setApplicationName("Sprocket");

		{
			QSurfaceFormat format;
			format.setSamples(4);

#ifndef CONVERT_TO_SRGB_IN_SHADER
			format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
#endif
			format.setDepthBufferSize(24);

			format.setProfile(QSurfaceFormat::CompatibilityProfile);

			QSurfaceFormat::setDefaultFormat(format);
		}

		std::string pluginsDir = (getExecutablePath() / "plugins").string();
		std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(getAllPluginFilepathsInDirectory(pluginsDir));
		std::vector<EditorPluginFactory> editorPluginFactories = loadPluginFactories<EditorPlugin, EditorPluginConfig>(getAllPluginFilepathsInDirectory(pluginsDir));
		Application application(enginePluginFactories, editorPluginFactories, argc, argv);

		if (!applicationSupportsOpenGl())
		{
			displayApplicationError("This program requires OpenGL to run.");
			return 1;
		}

		return application.exec();
	}
	catch (const std::exception &e)
	{
		displayApplicationError(e.what());
	}
	catch (...)
	{
		displayApplicationError("Exception caught in main");
	}

	return 0; 
}
