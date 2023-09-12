/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/ApplicationUtil.h>
#include <Sprocket/DarkStyle.h>
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/EngineSettingsSerialization.h>
#include <Sprocket/MainWindow.h>
#include <Sprocket/MainWindowUtil.h>
#include <Sprocket/SimUpdater.h>
#include <Sprocket/ContextAction/CreateContextActions.h>
#include <Sprocket/Input/ViewportInputSystem.h>
#include <Sprocket/Input/InputPlatformQt.h>
#include <Sprocket/Scenario/EntityObjectType.h>
#include <Sprocket/Viewport/ScenarioObjectPicker.h>
#include "Sprocket/Scenario/EntityObjectType.h"
#include "Sprocket/Scenario/ScenarioDescObjectType.h"
#include <Sprocket/Scenario/ScenarioSelectionModel.h>
#include <Sprocket/Viewport/DefaultViewportMouseEventHandler.h>
#include <Sprocket/Viewport/ViewportVisibilityFiltering.h>
#include <Sprocket/Widgets/ScenarioPropertyEditorWidget.h>
#include <Sprocket/Widgets/ScenarioObjectsEditorWidget.h>
#include <Sprocket/Widgets/TimelineControlWidget.h>
#include <Sprocket/Widgets/TimeControlWidget.h>
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

#include <QApplication>
#include <QMenu>

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
	// TODO: change to drawing on background thread. Need to ensure thread safety.
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

		mVisRoot = createVisRoot(*engineRoot);

		{
			EditorPluginConfig config;
			config.uiController = std::make_shared<UiController>();
			config.uiController->toolWindowRaiser = [this](QWidget* widget) {
				mMainWindow->raiseToolWindow(widget);
			};
			config.engineRoot = engineRoot.get();
			config.selectionModel = selectionModel;
			config.inputPlatform = inputPlatform;
			config.visRoot = mVisRoot.get();
			config.mainWindow = mMainWindow.get();

			mEditorPlugins = loadEditorPlugins(editorPluginFactories, config);
			addToolWindows(*mMainWindow, mEditorPlugins);
		}

		ScenarioObjectTypeMap scenarioObjectTypes = getSceneObjectTypes(mEditorPlugins, engineRoot.get());
		scenarioObjectTypes[typeid(EntityObject)] = createEntityObjectType(&engineRoot->scenario->world, engineRoot->entityFactory.get());
		scenarioObjectTypes[typeid(ScenarioDescObject)] = createScenarioDescObjectType(engineRoot->scenario.get());

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

		ViewportWidget* viewportWidget;
		{
			viewportWidget = new ViewportWidget([&] {
				ViewportWidgetConfig c;
				c.engineRoot = engineRoot.get();
				c.visRoot = mVisRoot;
				c.selectionModel = selectionModel;
				c.scenarioObjectPicker = createScenarioObjectPicker(scenarioObjectTypes);
				c.contextActions = createContextActions(*engineRoot);
				c.projectFilenameGetter = [this] { return mMainWindow->getProjectFilename().toStdString(); };
				c.parent = mMainWindow.get();
				return c;
			}());
			viewportWidget->addMouseEventHandler(viewportMouseEventHandler);

			EntityVisibilityPredicate basePredicate = createSelectedEntityVisibilityPredicateAndAddSubMenu(*viewportWidget->getVisibilityFilterMenu(), "See Through Planet", selectionModel);
			basePredicate = predicateOr(basePredicate, createLineOfSightVisibilityPredicate(viewportWidget, &engineRoot->scenario->world));
			addVisibilityLayerSubMenus(*viewportWidget->getVisibilityFilterMenu(), basePredicate, getEntityVisibilityLayers(mEditorPlugins), selectionModel);

			mMainWindow->addToolWindow("Viewport", viewportWidget);
			connectJsonProjectSerializable(*mMainWindow, *viewportWidget);
		}
		TimelineControlWidget* timeControlWidget;
		{
			timeControlWidget = new TimelineControlWidget(&engineRoot->scenario->timeSource, &engineRoot->scenario->timelineMode, mMainWindow.get());
			mMainWindow->addToolWindow("Time Control", timeControlWidget);
		}

		mSimUpdater = std::make_unique<SimUpdater>(engineRoot);

		QObject::connect(mMainWindow.get(), &MainWindow::updated, viewportWidget, [this, timeControlWidget, viewportWidget] (sim::SecondsD wallDt) {
			mSimUpdater->setRequestedTimeRate(timeControlWidget->getTimeControlWidget()->getRequestedTimeRate());
			mSimUpdater->update(wallDt);
			
			timeControlWidget->getTimeControlWidget()->setActualTimeRate(mSimUpdater->getActualTimeRate());
			viewportWidget->update();

			mVisRoot->render();
		});

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
	vis::VisRootPtr mVisRoot;
	std::unique_ptr<SimUpdater> mSimUpdater;
	std::unique_ptr<MainWindow> mMainWindow;
	std::vector<EditorPluginPtr> mEditorPlugins;
};

int main(int argc, char *argv[])
{
	try
	{
		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
		QCoreApplication::setApplicationName("Sprocket");

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
