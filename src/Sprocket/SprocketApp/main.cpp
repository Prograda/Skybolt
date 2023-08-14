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
#include <Sprocket/SceneSelectionModel.h>
#include <Sprocket/ContextAction/CreateContextActions.h>
#include <Sprocket/Input/EditorInputSystem.h>
#include <Sprocket/Input/InputPlatformOis.h>
#include <Sprocket/Scenario/EntityObjectType.h>
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

		auto selectionModel = new SceneSelectionModel(mMainWindow.get());

		auto inputPlatform = std::make_shared<InputPlatformOis>(std::to_string(size_t(mMainWindow->winId())), 800, 600); // TODO: get actual dimensions, which are currently only needed on Linux
		auto inputSystem = std::make_shared<EditorInputSystem>(inputPlatform);
		engineRoot->systemRegistry->insert(engineRoot->systemRegistry->begin(), inputSystem);

		vis::VisRootPtr visRoot = createVisRoot(*engineRoot);

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
			auto window = new ScenarioPropertyEditorWidget([&] {
				ScenarioPropertyEditorWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.selectionModel = selectionModel;
				c.propertyModelFactoryMap = getPropertyModelFactories(mEditorPlugins, engineRoot.get());
				c.widgetFactoryMap = getPropertyEditorWidgetFactories(mEditorPlugins);
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Properties", window);
		}
		{
			auto window = new ScenarioObjectsEditorWidget([&] {
				ScenarioObjectsEditorWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.selectionModel = selectionModel;
				c.scenarioObjectTypes = scenarioObjectTypes;
				c.contextActions = createContextActions(*engineRoot);
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Explorer", window);
		}
		{
			auto entityObjectRegistry = findOptional(scenarioObjectTypes, std::type_index(typeid(EntityObject)));
			assert(entityObjectRegistry);

			auto window = new ViewportWidget([&] {
				ViewportWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.visRoot = visRoot;
				c.viewportInput = inputSystem->getViewportInput();
				c.selectionModel = selectionModel;
				c.contextActions = createContextActions(*engineRoot);
				c.projectFilenameGetter = [this] { return mMainWindow->getProjectFilename().toStdString(); };
				c.parent = mMainWindow.get();
				return c;
			}());
			window->setMouseEventHandler(std::make_shared<DefaultViewportMouseEventHandler>([&] {
				DefaultViewportMouseEventHandlerConfig c;
				c.viewportWidget = window;
				c.viewportInput = inputSystem->getViewportInput();
				c.entityObjectRegistry = (*entityObjectRegistry)->objectRegistry;
				c.sceneSelectionModel = selectionModel;
				return c;
			}()));

			for (const auto& layer : getEntityVisibilityLayers(mEditorPlugins))
			{
				window->addVisibilityFilterableSubMenu(QString::fromStdString(layer.first), layer.second);
			}

			mMainWindow->addToolWindow("Viewport", window);

			QObject::connect(mMainWindow.get(), &MainWindow::updated, window, &ViewportWidget::update);
			connectJsonProjectSerializable(*mMainWindow, *window);
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
				mMainWindow->open(filename);
			}
			else
			{
				mMainWindow->newScenario();
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
