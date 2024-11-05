/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltQt/MainWindow.h>
#include <SkyboltQt/ContextAction/CreateContextActions.h>
#include <SkyboltQt/Engine/EngineSettingsSerialization.h>
#include <SkyboltQt/Engine/FindPython.h>
#include <SkyboltQt/Engine/SimUpdater.h>
#include <SkyboltQt/Input/ViewportInputSystem.h>
#include <SkyboltQt/Input/InputPlatformQt.h>
#include <SkyboltQt/Property/DefaultPropertyModelFactories.h>
#include <SkyboltQt/QtUtil/ApplicationUtil.h>
#include <SkyboltQt/QtUtil/QtLayoutUtil.h>
#include <SkyboltQt/QtUtil/QtMenuUtil.h>
#include <SkyboltQt/QtUtil/QtTimerUtil.h>
#include "SkyboltQt/Scenario/EntityObjectType.h"
#include "SkyboltQt/Scenario/ScenarioDescObjectType.h"
#include <SkyboltQt/Scenario/ScenarioSelectionModel.h>
#include "SkyboltQt/Scenario/ScenarioWorkspace.h"
#include <SkyboltQt/Style/DarkStyle.h>
#include <SkyboltQt/ThirdParty/DarkTitleBar.h>
#include <SkyboltQt/Viewport/DefaultViewportMouseEventHandler.h>
#include <SkyboltQt/Viewport/ScenarioObjectPicker.h>
#include <SkyboltQt/Viewport/ViewportVisibilityFiltering.h>
#include <SkyboltQt/Viewport/VisSelectionIcons.h>
#include <SkyboltQt/Widgets/EngineSystemsWidget.h>
#include <SkyboltQt/Widgets/EntityControllerWidget.h>
#include <SkyboltQt/Widgets/ErrorLogModel.h>
#include <SkyboltQt/Widgets/ScenarioPropertyEditorWidget.h>
#include <SkyboltQt/Widgets/ScenarioObjectsEditorWidget.h>
#include <SkyboltQt/Widgets/StatusBar.h>
#include <SkyboltQt/Widgets/TimelineControlWidget.h>
#include <SkyboltQt/Widgets/TimeControlWidget.h>
#include <SkyboltQt/Widgets/ViewportToolBar.h>
#include <SkyboltQt/Widgets/ViewportWidget.h>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Stringify.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/GetExecutablePath.h>
#include <SkyboltEngine/Diagnostics/StatsDisplaySystem.h>
#include <SkyboltCommon/Logging/ConsoleSink.h>
#include <SkyboltEngine/Input/InputSystem.h>
#include <SkyboltEngine/SimVisBinding/ForcesVisBinding.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/SimVisBinding/VisNameLabels.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Renderable/Arrows.h>
#include <SkyboltVis/RenderOperation/RenderOperationUtil.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Shader/ShaderSourceFileChangeMonitor.h>
#include <SkyboltVis/Window/Window.h>

#include <QApplication>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSplashScreen>
#include <QVBoxLayout>

using namespace skybolt;

static vis::VisRootPtr createVisRoot(const EngineRoot& engineRoot)
{
	auto visRoot = std::make_shared<vis::VisRoot>(getDisplaySettingsFromEngineSettings(engineRoot.engineSettings));
	// TODO: change to drawing on background thread, but first we need to ensure thread safety.
	visRoot->getViewer().setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	return visRoot;
}

static EntityVisibilityPredicateSetter toEntityVisibilityPredicateSetter(EntityVisibilityFilterable* filterable)
{
	return [filterable] (EntityVisibilityPredicate p) { filterable->setEntityVisibilityPredicate(std::move(p)); };
}

static std::unique_ptr<QSplashScreen> createSplashScreen(const EngineRoot& engineRoot)
{
	std::optional<file::Path> splashFile = skybolt::valueOrLogError(engineRoot.fileLocator("Ui/Splash.jpg"));
	QPixmap pixmap = splashFile ? QPixmap(QString::fromStdString(splashFile->string())) : QPixmap();
	auto splash = std::make_unique<QSplashScreen>(pixmap);

#ifdef SKYBOLT_VERSION
	QString versionNumber = QString::fromUtf8(STRINGIFY(SKYBOLT_VERSION));
#else
	QString versionNumber = "???";
#endif
	auto versionLabel = new QLabel("Version " + versionNumber, splash.get());
	QFont font = versionLabel->font();
	font.setPointSizeF(15);
	versionLabel->setFont(font);
	versionLabel->move(pixmap.width() * (250.f / 1240.f), pixmap.height() * (470.f / 600.f));

	return splash;
}

static void setSplashScreenMessage(QSplashScreen& screen, const QString& message)
{
	screen.showMessage(message, Qt::AlignBottom, Qt::white);
}

class Application : public QApplication
{
public:
	Application(const std::vector<PluginFactory>& enginePluginFactories, int argc, char* argv[]) :
		QApplication(argc, argv)
	{
		setStyle(new DarkStyle);

		// Create model for logging application warnings and errors
		auto errorLogModel = new ErrorLogModel(this);
		connectToBoostLogger(errorLogModel);

		// Create engine
		{
			QSettings settings(QApplication::applicationName());
			nlohmann::json engineSettings = readOrCreateEngineSettingsFile(settings);
			EngineRootConfig config;
			config.engineSettings = engineSettings;
			mEngineRoot = std::make_unique<EngineRoot>(config);
		}

#ifdef PYTHON_VERSION_MAJOR
		// Warn user if python is not available
		if (!isPythonOnPath(PYTHON_VERSION_MAJOR, PYTHON_VERSION_MINOR))
		{
			BOOST_LOG_TRIVIAL(warning) << QString("Python %1.%2 not found in PATH environment variable. Python functionality will be disabled.")
				.arg(PYTHON_VERSION_MAJOR).arg(PYTHON_VERSION_MINOR).toStdString();
		}
#endif

		// Create splash screen
		mSplashScreen = createSplashScreen(*mEngineRoot);
		mSplashScreen->show();

		// Load plugins
		setSplashScreenMessage(*mSplashScreen, "Loading plugins...");
		mEngineRoot->loadPlugins(enginePluginFactories);

		// Create visualization
		mVisRoot = createVisRoot(*mEngineRoot);

		// Create user input system
		std::shared_ptr<ViewportInputSystem> viewportInputSystem;
		{
			auto inputPlatform = std::make_shared<InputPlatformQt>();
			CameraInputAxes axes = createDefaultCameraInputAxes(*inputPlatform);
			mEngineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, toValuesVector(axes)));

			viewportInputSystem = std::make_shared<ViewportInputSystem>(inputPlatform, axes);
			mEngineRoot->systemRegistry->push_back(viewportInputSystem);
		}

		// Create scenario object types
		auto scenarioObjectTypes = std::make_shared<ScenarioObjectTypeMap>(ScenarioObjectTypeMap{
			{ typeid(EntityObject), createEntityObjectType(&mEngineRoot->scenario->world, mEngineRoot->entityFactory.get()) },
			{ typeid(ScenarioDescObject), createScenarioDescObjectType(mEngineRoot->scenario.get()) }
		});

		// Create viewport overlays
		{
			sim::World* world = &mEngineRoot->scenario->world;
			auto hudGroup = mEngineRoot->scene->getBucketGroup(vis::Scene::Bucket::Hud);

			auto simVisSystem = sim::findSystem<SimVisSystem>(*mEngineRoot->systemRegistry);

			// Create selection icons
			mVisSelectionIcons = std::make_shared<VisSelectionIcons>(hudGroup, mEngineRoot->programs);
			simVisSystem->addBinding(mVisSelectionIcons);

			// Create entity name labels
			mVisNameLabels = std::make_shared<VisNameLabels>(world, hudGroup, mEngineRoot->programs);
			simVisSystem->addBinding(mVisNameLabels );

			// Create arrows to visualize physics forces
			mForcesVisBinding = createForcesVisBinding(world, mEngineRoot->scene, mEngineRoot->programs);
			simVisSystem->addBinding(mForcesVisBinding);
		}

		// Create Main window
		auto scenarioWorkspace = std::make_shared<ScenarioWorkspace>(mEngineRoot);

		mMainWindow.reset(new MainWindow([&] {
			MainWindowConfig c;
			c.workspace = scenarioWorkspace;
			c.engineRoot = mEngineRoot;
			return c;
		}()));
		addErrorLogStatusBar(*mMainWindow->statusBar(), errorLogModel);
		enableDarkTitleBar(mMainWindow->winId());

		auto selectionModel = new ScenarioSelectionModel(mMainWindow.get());

		setSplashScreenMessage(*mSplashScreen, "Initializing tool windows...");

		// Create property editor
		{
			auto widget = new ScenarioPropertyEditorWidget([&] {
				ScenarioPropertyEditorWidgetConfig c;
				c.engineRoot = mEngineRoot.get();
				c.selectionModel = selectionModel;
				c.propertyModelFactoryMap = getDefaultPropertyModelFactories(mEngineRoot.get());
				c.widgetFactoryMap = getDefaultEditorWidgetFactoryMap();
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Properties", widget);
		}

		// Create objects editor
		{
			auto widget = new ScenarioObjectsEditorWidget([&] {
				ScenarioObjectsEditorWidgetConfig c;
				c.engineRoot = mEngineRoot.get();
				c.selectionModel = selectionModel;
				c.scenarioObjectTypes = *scenarioObjectTypes;
				c.contextActions = createContextActions(*mEngineRoot);
				c.parent = mMainWindow.get();
				return c;
			}());
			mMainWindow->addToolWindow("Explorer", widget);
		}

		// Create viewport mouse handler
		auto viewportMouseEventHandler = std::make_shared<DefaultViewportMouseEventHandler>([&] {
				DefaultViewportMouseEventHandlerConfig c;
				c.engineRoot = mEngineRoot.get();
				c.viewportInput = viewportInputSystem;
				c.scenarioSelectionModel = selectionModel;
				c.viewportSelectionPredicate = [] (const ViewportWidget&) { return ScenarioObjectPredicateAlways; };
				return c;
			}());

		// Create viewport
		ViewportWidget* viewportWidget;
		{
			viewportWidget = new ViewportWidget([&] {
				ViewportWidgetConfig c;
				c.engineRoot = mEngineRoot.get();
				c.visRoot = mVisRoot;
				c.selectionModel = selectionModel;
				c.scenarioObjectPicker = createScenarioObjectPicker(*scenarioObjectTypes);
				c.contextActions = createContextActions(*mEngineRoot);
				c.parent = mMainWindow.get();
				return c;
			}());
			viewportWidget->addMouseEventHandler(viewportMouseEventHandler);

			auto viewportToolBar = new ViewportToolBar([&] {
				ViewportToolBarConfig c;
				c.engineRoot = mEngineRoot.get();
				c.viewport = viewportWidget;
				c.scenarioFilenameGetter = [scenarioWorkspace] { return scenarioWorkspace->getScenarioFilename().toStdString(); };
				c.parent = mMainWindow.get();
				return c;
				}());

			EntityVisibilityPredicate basePredicate = createSelectedEntityVisibilityPredicateAndAddSubMenu(*viewportToolBar->getVisibilityFilterMenu(), "See Through Planet", selectionModel);
			basePredicate = predicateOr(basePredicate, createLineOfSightVisibilityPredicate(viewportWidget, &mEngineRoot->scenario->world));

			auto visibilityLayers = std::map<std::string, EntityVisibilityPredicateSetter>({
				{"Labels", toEntityVisibilityPredicateSetter(mVisNameLabels.get())},
				{"Forces", toEntityVisibilityPredicateSetter(mForcesVisBinding.get())}
			});

			addVisibilityLayerSubMenus(*viewportToolBar->getVisibilityFilterMenu(), basePredicate, visibilityLayers, selectionModel);

			auto viewportToolWidget = new QWidget(mMainWindow.get());
			createBoxLayoutWithWidgets({ viewportToolBar, viewportWidget }, viewportToolWidget, QBoxLayout::Direction::TopToBottom);

			mMainWindow->addToolWindow("Viewport", viewportToolWidget);
			connectJsonScenarioSerializable(*scenarioWorkspace, viewportWidget);
		}

		// Create timeline widget
		auto requestedTimeRate = std::make_shared<ObservableValueD>(1.0);
		auto actualTimeRate = std::make_shared<ObservableValueD>(1.0);
		{
			auto timeControlWidget = new TimelineControlWidget([&] {
				TimelineControlWidgetConfig c;
				c.timeSource = &mEngineRoot->scenario->timeSource;
				c.timelineMode = &mEngineRoot->scenario->timelineMode;
				c.requestedTimeRate = requestedTimeRate.get();
				c.actualTimeRate = actualTimeRate.get();
				c.parent = mMainWindow.get();
				return c;
				}());
			mMainWindow->addToolWindow("Time Control", timeControlWidget);
		}

		// Create entity controller widget
		{
			auto entityControllerWidget = new EntityControllerWidget(mMainWindow.get());

			QObject::connect(selectionModel, &ScenarioSelectionModel::selectionChanged, [this, selectionModel, entityControllerWidget] (const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected) {
				if (auto entityObject = getFirstSelectedScenarioObjectOfType<EntityObject>(selected); entityObject)
				{
					sim::Entity* entity = mEngineRoot->scenario->world.getEntityById(entityObject->data).get();
					entityControllerWidget->setEntity(entity);
				}
				else
				{
					entityControllerWidget->setEntity(nullptr);
				}
				mVisSelectionIcons->setSelection(selectionModel->getSelectedItems());
			});

			mMainWindow->addToolWindow("Entity Controller", entityControllerWidget);
		}

		addDeveloperMenu();

		// Begin update timer
		mSimUpdater = std::make_unique<SimUpdater>(mEngineRoot);

		createAndStartIntervalDtTimer(10, mMainWindow.get(), [this, requestedTimeRate, actualTimeRate, viewportWidget] (sim::SecondsD wallDt) {
			mSimUpdater->setRequestedTimeRate(requestedTimeRate->get());
			mSimUpdater->update(wallDt);
			
			actualTimeRate->set(mSimUpdater->getActualTimeRate());
			viewportWidget->update();

			if (mShaderSourceFileChangeMonitor)
			{
				mShaderSourceFileChangeMonitor->update();
			}

			mVisRoot->render();
		});

		// Load default window state
		const QString defaultWindowLayoutState = "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";
		mMainWindow->restoreToolWindowsState(defaultWindowLayoutState);

		// Open scenario if one was given on commandline
		try
		{
			if (argc >= 2)
			{
				setSplashScreenMessage(*mSplashScreen, "Loading scenario...");

				QString filename(argv[1]);
				mMainWindow->openScenario(filename, MainWindow::OverwriteMode::OverwriteWithoutPrompt);
			}
		}
		catch (...)
		{
			mMainWindow.reset();
			throw;
		}

		mSplashScreen->finish(mMainWindow.get());

		mMainWindow->showMaximized();
	}

	~Application()
	{
		mMainWindow.reset();
	}

protected:
	void addDeveloperMenu()
	{
		QMenuBar* menuBar = mMainWindow->menuBar();
		QMenu* devMenu = new QMenu("Developer", menuBar);
		insertMenuBefore(*menuBar, "Tools", *devMenu);

		{
			QAction* action = devMenu->addAction("Viewport Stats");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setStatsDisplaySystemEnabled(visible); });
		}
		{
			QAction* action = devMenu->addAction("Viewport Textures");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setViewportTextureDisplayEnabled(visible); });
		}
		{
			QAction* action = devMenu->addAction("Live Shader Editing");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setLiveShaderEditingEnabled(visible); });
		}
		{
			QAction* action = devMenu->addAction("View Systems...", [this](bool visible) { showSystemList(); });
		}
	}

	void acquireViewport()
	{
		mWindow = mVisRoot->getWindows().empty() ? nullptr : mVisRoot->getWindows().front();
		if (mWindow)
		{
			auto ops = mWindow->getRenderOperationSequence().findOperationsOfType<vis::RenderCameraViewport>();
			mViewport = ops.empty() ? nullptr : ops.back();
		}
	}

	void setStatsDisplaySystemEnabled(bool enabled)
	{
		acquireViewport();
		if (mWindow && mViewport)
		{
			if (!mStatsDisplaySystem)
			{
				mStatsDisplaySystem = std::make_shared<StatsDisplaySystem>(&mVisRoot->getViewer(), mWindow->getView(), mViewport->getFinalRenderTarget()->getOsgCamera());
				mStatsDisplaySystem->setVisible(false);
				mEngineRoot->systemRegistry->push_back(mStatsDisplaySystem);
			}

			mStatsDisplaySystem->setVisible(enabled);
		}
	}

	void setViewportTextureDisplayEnabled(bool enabled)
	{
		acquireViewport();
		if (mWindow && mViewport)
		{
			if (enabled && !mRenderOperationVisualization)
			{
				mRenderOperationVisualization = vis::createRenderOperationVisualization(mViewport, mEngineRoot->programs);
				mWindow->getRenderOperationSequence().addOperation(mRenderOperationVisualization);
			}
			else if (!enabled && mRenderOperationVisualization)
			{
				mWindow->getRenderOperationSequence().removeOperation(mRenderOperationVisualization);
			}
		}
	}

	void setLiveShaderEditingEnabled(bool enabled)
	{
		mShaderSourceFileChangeMonitor.reset();
		if (enabled)
		{
			mShaderSourceFileChangeMonitor = std::make_unique<vis::ShaderSourceFileChangeMonitor>(mEngineRoot->programs);
		}
	}

	void showSystemList()
	{
		QDialog dialog;
		dialog.setWindowTitle("System List");
		QVBoxLayout layout(&dialog);

		layout.addWidget(createEngineSystemsWidget(*mEngineRoot->systemRegistry, mMainWindow.get()));

		auto closeButton = new QPushButton("Close", &dialog);
		layout.addWidget(closeButton);
		QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

		dialog.exec();
	}

private:
	vis::VisRootPtr mVisRoot;
	std::unique_ptr<SimUpdater> mSimUpdater;
	std::unique_ptr<MainWindow> mMainWindow;
	std::shared_ptr<EngineRoot> mEngineRoot;
	std::unique_ptr<QSplashScreen> mSplashScreen;
	vis::WindowPtr mWindow;
	osg::ref_ptr<skybolt::vis::RenderCameraViewport> mViewport;
	std::shared_ptr<skybolt::StatsDisplaySystem> mStatsDisplaySystem;
	std::unique_ptr<skybolt::vis::ShaderSourceFileChangeMonitor> mShaderSourceFileChangeMonitor;
	osg::ref_ptr<skybolt::vis::RenderOperation> mRenderOperationVisualization;
	std::shared_ptr<skybolt::VisSelectionIcons> mVisSelectionIcons;
	std::shared_ptr<skybolt::VisNameLabels> mVisNameLabels;
	std::shared_ptr<skybolt::ForcesVisBinding> mForcesVisBinding;
};

int main(int argc, char *argv[])
{
	try
	{
		file::Path executableDir = getExecutablePath();

		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
		QCoreApplication::setApplicationName("SkyboltQt");
		QCoreApplication::addLibraryPath(QString::fromStdString((executableDir / "qtPlugins").string()));

		addConsoleLogSink();

		std::string pluginsDir = (executableDir / "plugins").string();
		std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(getAllPluginFilepathsInDirectory(pluginsDir));

		try
		{
			Application application(enginePluginFactories, argc, argv);

			if (!applicationSupportsOpenGl())
			{
				displayApplicationError("This program requires OpenGL to run.");
				return 1;
			}

			try
			{
				return application.exec();
			}
			catch (const std::exception &e)
			{
				// Copy and re-throw exception, as the original exception won't exist anymore if python thew an exception,
				// since application will be destroyed
				throw Exception(e.what());
			}
		}
		catch (const std::exception &e)
		{
			// Copy and re-throw exception, as the original exception won't exist anymore if plugin thew an exception,
			// since enginePluginFactories will be destroyed
			throw Exception(e.what());
		}
	}
	catch (const std::exception &e)
	{
		displayApplicationError(e.what());
	}
	catch (...)
	{
		displayApplicationError("Exception caught in main");
	}

	return EXIT_FAILURE; 
}
