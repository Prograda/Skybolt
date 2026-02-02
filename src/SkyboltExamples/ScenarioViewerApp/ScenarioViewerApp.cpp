#include "CameraControllerWidget.h"
#include "EntityCreationToolBar.h"
#include "MainWindow.h"
#include "ScenarioTreeWidget.h"
#include "ScenarioWorkspace.h"

#include <SkyboltEngine/Diagnostics/StatsDisplaySystem.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/GetExecutablePath.h>
#include <SkyboltEngine/Input/InputSystem.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/UpdateLoop/SimUpdater.h>
#include <SkyboltEngine/WindowUtil.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/Scenario/SimSnapshotRegistry.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/VisNameLabels.h>
#include <SkyboltEngineQt/OsgWindow.h>
#include <SkyboltEngineQt/SkyboltQtPropertyReflection.h>
#include <SkyboltEngineQt/Icon/SkyboltIcons.h>
#include <SkyboltEngineQt/Input/InputPlatformQt.h>
#include <SkyboltEngineQt/Input/ViewportInputSystem.h>
#include <SkyboltEngineQt/Property/EntityPropertiesModel.h>
#include <SkyboltEngineQt/Property/ScenarioPropertiesModel.h>
#include <SkyboltEngineQt/ThirdParty/DarkTitleBar.h>
#include <SkyboltEngineQt/Style/DarkStyle.h>
#include <SkyboltEngineQt/Widgets/ErrorLogModelUtil.h>
#include <SkyboltEngineQt/Widgets/SkyboltEditorWidgets.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Window/Window.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationUtil.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Shader/ShaderSourceFileChangeMonitor.h>
#include <SkyboltWidgets/CollapsiblePanel/CollapsiblePanelWidget.h>
#include <SkyboltWidgets/List/ListEditorWidget.h>
#include <SkyboltWidgets/ErrorLog/ErrorLogModel.h>
#include <SkyboltWidgets/ErrorLog/ErrorLogWidget.h>
#include <SkyboltWidgets/ErrorLog/LatestErrorWidget.h>
#include <SkyboltWidgets/Property/PropertyEditor.h>
#include <SkyboltWidgets/Property/EditorWidgets.h>
#include <SkyboltWidgets/Property/DefaultEditorWidgets.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltWidgets/Property/QtPropertyReflectionConversion.h>
#include <SkyboltWidgets/Property/QtMetaTypes.h>
#include <SkyboltWidgets/Timeline/TimeControlWidget.h>
#include <SkyboltWidgets/Timeline/TimelineWidget.h>
#include <SkyboltWidgets/Timeline/TimeRateDialog.h>
#include <SkyboltWidgets/Util/QtLayoutUtil.h>
#include <SkyboltWidgets/Util/QtScrollAreaUtil.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>
#include <SkyboltWidgets/Util/RecentFilesMenuPopulator.h>

#include <QApplication>
#include <QSplashScreen>
#include <QTreeView>
#include <QVBoxLayout>

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>
#include <cxxtimer/cxxtimer.hpp>

using namespace skybolt;
using namespace skybolt::sim;

static TimeControlWidgetIcons createSkyboltTimeControlWidgetIcons()
{
	QStyle* style = QApplication::style();

	TimeControlWidgetIcons icons;
	icons.play = getSkyboltIcon(SkyboltIcon::Play);
	icons.pause = getSkyboltIcon(SkyboltIcon::Pause);
	icons.forward = getSkyboltIcon(SkyboltIcon::SkipForward);
	icons.backward = getSkyboltIcon(SkyboltIcon::SkipBackward);

	return icons;
}

static QString toTimeText(double time)
{
	QString str = QDateTime::fromMSecsSinceEpoch(time * 1000).toUTC().toString("hh:mm:ss.zzz");
	return str;
}

static QWidget* createTimeControlWidgetPanel(NonNullPtr<EngineRoot> engineRoot, NonNullPtr<SimUpdater> simUpdater, QWidget* parent)
{
	auto widget = new QWidget(parent);
	auto layout = new QVBoxLayout(widget);

	auto timeControl = new TimeControlWidget(createSkyboltTimeControlWidgetIcons(), parent);
	layout->addWidget(timeControl);

	auto timeline = new TimelineWidget(widget);
	auto currentTimeLabel = new QLabel(widget);
	auto durationLabel = new QLabel(widget);
	{
		auto timelineLayout = new QHBoxLayout();
		timelineLayout->addWidget(currentTimeLabel);
		timelineLayout->addWidget(timeline);
		timelineLayout->addWidget(durationLabel);
		layout->addLayout(timelineLayout);
	}

	QObject::connect(timeControl, &TimeControlWidget::requestedPlayStateChanged, widget, [engineRoot](bool playing) {
		return engineRoot->scenario->timeSource.setState(playing ? TimeSource::StatePlaying : TimeSource::StateStopped);
		});

	QObject::connect(timeControl, &TimeControlWidget::requestedTimeForward, widget, [engineRoot, timeline]() {
		auto& timeSource = engineRoot->scenario->timeSource;
		timeSource.setTime(timeSource.getRange().end);
		});

	QObject::connect(timeControl, &TimeControlWidget::requestedTimeBackward, widget, [engineRoot, timeline]() {
		auto& timeSource = engineRoot->scenario->timeSource;
		timeSource.setTime(timeSource.getRange().start);
		});

	QObject::connect(timeline, &TimelineWidget::timeChanged, widget, [engineRoot](double t) {
		engineRoot->scenario->timeSource.setTime(t);
	});

	auto timeRateDialog = new TimeRateDialog(simUpdater->getRequestedTimeRate(), widget);
	addTimeRateDialogToToolBar(timeRateDialog, timeControl->getToolBar(), getSkyboltIcon(SkyboltIcon::Speed));

	QObject::connect(timeRateDialog, &TimeRateDialog::rateChanged, widget, [simUpdater](double timeRate) {
		simUpdater->setRequestedTimeRate(timeRate);
		});

	createAndStartIntervalTimer(10, parent, [engineRoot, timeControl, timeline, currentTimeLabel, durationLabel] () {
		auto& timeSource = engineRoot->scenario->timeSource;
		timeControl->setPlaying(timeSource.getState() == TimeSource::StatePlaying);
		timeline->setTime(timeSource.getTime());
		timeline->setRange({timeSource.getRange().start, timeSource.getRange().end});
		timeline->setUserTimeChangeAllowed(engineRoot->scenario->timelineMode.get() == TimelineMode::Free);
		timeControl->setForwardEnabled(engineRoot->scenario->timelineMode.get() == TimelineMode::Free);
		currentTimeLabel->setText(toTimeText(timeSource.getTime()));
		durationLabel->setText(toTimeText(timeSource.getRange().end - timeSource.getRange().start));
	});

	return widget;
}

static QTimer* createAndStartIntervalDtTimer(int intervalMilliseconds, QObject* parent, std::function<void(skybolt::sim::SecondsD dt)> updateAction)
{
	return createAndStartIntervalTimer(intervalMilliseconds, parent, [updateAction = std::move(updateAction), timeSinceLastUpdate = cxxtimer::Timer()] () mutable {
		double elapsed = double(timeSinceLastUpdate.count<std::chrono::milliseconds>()) / 1000.0;

		timeSinceLastUpdate.reset();
		timeSinceLastUpdate.start();

		updateAction(elapsed);
	});
}

#ifdef WIN32
#include <windows.h>
#endif

static void displayApplicationError(const std::string &error)
{
	printf("Error: %s\n", error.c_str());
	#ifdef WIN32
	MessageBox(NULL, std::wstring(error.begin(), error.end()).c_str(), L"Exception", 0);
	#endif
}

static std::unique_ptr<QSplashScreen> createSplashScreen()
{
	QString splashScreenFilename = ":/Resources/Splash.png";
	if (!QFile::exists(splashScreenFilename))
	{
		BOOST_LOG_TRIVIAL(error) << "Splash screen image not found: '" << splashScreenFilename.toStdString() << "'";
	}
	QPixmap pixmap(splashScreenFilename);
	auto splash = std::make_unique<QSplashScreen>(pixmap);

	auto label = new QLabel("Scenario Viewer", splash.get());
	QFont font = label->font();
	font.setPointSizeF(18);
	label->setFont(font);
	label->move(pixmap.width() * (250.f / 1240.f), pixmap.height() * (465.f / 600.f));

	return splash;
}

static void setSplashScreenMessage(QSplashScreen& screen, const QString& message)
{
	screen.showMessage(message, Qt::AlignBottom, Qt::white);
}

class ApplicationWithErrorHandling : public QApplication
{
public:
	using QApplication::QApplication;

	bool notify(QObject* receiver, QEvent* event) override
    {
		try
		{
			return QApplication::notify(receiver, event);
		}
		catch (const std::exception& e)
		{
            displayApplicationError(e.what());
		}        
		return false;
     }
};

static void placeEntityOnPlanetInFrontOfCamera(const Entity& camera, Entity& entityToBePlaced)
{
	if (entityToBePlaced.getFirstComponent<PlanetComponent>())
	{
		// Leave planet at origin
		return;
	}

	// Get a position in front of the camera
	Vector3 position = getPosition(camera).value_or(math::dvec3Zero());
	Quaternion orientation = getOrientation(camera).value_or(Vector3(1,0,0));
	Vector3 direction = orientation * Vector3(1,0,0);
	double distance = 100;
	position += direction * distance;

	// Snap orientation to have zero pitch and roll relative to planet surface
	Vector3 upDirection = math::normalizeSafe(position).value_or(Vector3(1,0,0));
	Vector3 outForward;
	Vector3 outRight;
	math::getOrthonormalBasis(-upDirection, direction, outForward, outRight);
	orientation = Matrix3(outForward, outRight, -upDirection);

	// Clamp altitude to inside atmosphere, in case camera is far from planet
	double radius = glm::length(position);
	radius = std::clamp(radius, earthRadius(), earthRadius() + 100000.0);
	position = upDirection * radius;

	// Set pose
	setPosition(entityToBePlaced, position + direction * distance);
	setOrientation(entityToBePlaced, orientation);
}

static QString getStatusBarText(const EngineRoot& engineRoot)
{
	QStringList status;
	if (engineRoot.stats.terrainTileLoadQueueSize)
	{
		status += "Loading terrain tiles: " + QString::number(engineRoot.stats.terrainTileLoadQueueSize);
	}

	if (engineRoot.stats.featureTileLoadQueueSize)
	{
		status += "Loading feature tiles: " + QString::number(engineRoot.stats.featureTileLoadQueueSize);
	}

	uint32_t activeTasks = engineRoot.scheduler->active_threads();
	if (activeTasks)
	{
		status += "Processing tasks: " + QString::number(activeTasks);
	}
	return status.join(",");
}

static int createAndExecuteApplication(int argc, char** argv)
{
	// Create application
	ApplicationWithErrorHandling app(argc, argv);
	app.setStyle(new DarkStyle);

	// Create splash screen
	std::unique_ptr<QSplashScreen> splashScreen = createSplashScreen();
	splashScreen->show();
	setSplashScreenMessage(*splashScreen, "Loading engine...");

	// Create model for logging application warnings and errors
	ErrorLogModel errorLogModel;
	connectToBoostLogger(&errorLogModel);

	// Create engine
	auto params = EngineCommandLineParser::parse(argc, argv);
	std::unique_ptr<EngineRoot> engineRoot = EngineRootFactory::create(params);

	SimUpdater simUpdater(engineRoot.get());

	// Allow user to reset the simulation state to t=0 if they rewind to the beginning
	std::shared_ptr<SimSnapshotRegistry> snapshotRegistry = createSnapshotRegistry(*engineRoot);
	useSnapshotsToResetSimulationStateAtTimelineStart(snapshotRegistry, engineRoot->scenario.get());

	setSplashScreenMessage(*splashScreen, "Loading UI...");

	// Build main window
	QSettings settings(QApplication::applicationName());
	MainWindow mainWindow(&settings);
	auto central = new QWidget(&mainWindow);
	mainWindow.setCentralWidget(central);
	enableDarkTitleBar(mainWindow.winId());

	// Use a grid layout to place widgets in multiple columns and rows
	auto mainLayout = new QVBoxLayout(central);

	// Create camera selector
	auto cameraControllerWidget = new CameraControllerWidget(&engineRoot->scenario->world, &mainWindow);
	mainLayout->addWidget(cameraControllerWidget);

	// Create 3D viewport
	auto visRoot = std::make_shared<skybolt::vis::VisRoot>();
	std::unique_ptr<OsgWindow> osgWindow = std::make_unique<OsgWindow>(visRoot);
	osg::ref_ptr<vis::RenderCameraViewport> viewport = createAndAddViewportToWindowWithEngine(*osgWindow->getWindow(), *engineRoot);
	{
		QWidget* osgContainer = QWidget::createWindowContainer(osgWindow.get(), &mainWindow);
		mainLayout->addWidget(osgContainer, 1);
	}

	// Create user input system
	auto inputPlatform = std::make_shared<InputPlatformQt>();
	std::shared_ptr<ViewportInputSystem> viewportInputSystem;
	{
		CameraInputAxes axes = createDefaultCameraInputAxes(*inputPlatform);
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, skybolt::toValuesVector(axes)));

		viewportInputSystem = std::make_shared<ViewportInputSystem>(inputPlatform, axes, engineRoot.get());
		engineRoot->systemRegistry->push_back(viewportInputSystem);

		QObject::connect(osgWindow.get(), &OsgWindow::mousePressed, [viewportInputSystem](const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) {
			viewportInputSystem->setMouseEnabled(true);
			viewportInputSystem->setKeyboardEnabled(true);
			});
	}

	auto viewportCamera = std::make_shared<sim::EntityId>();

	// Handle camera selection change event
	QObject::connect(cameraControllerWidget, &CameraControllerWidget::cameraSelectionChanged, [viewport, engineRoot = engineRoot.get(), viewportInputSystem, viewportCamera](const sim::Entity* camera) {
		if (camera)
		{
			World* world = &engineRoot->scenario->world;
			viewport->setCamera(getVisCamera(*camera));
			auto simVisSystem = sim::findRequiredSystem<SimVisSystem>(*engineRoot->systemRegistry);
			simVisSystem->setSceneOriginProvider(SimVisSystem::sceneOriginFromEntity(world, camera->getId()));
			connectToCameraExclusivly(*viewportInputSystem, world, camera->getId());
		}
		*viewportCamera = camera ? camera->getId() : sim::nullEntityId();
		});

	// Create time control panel
	{
		QWidget* timelinePanel = createTimeControlWidgetPanel(engineRoot.get(), &simUpdater, central);
		mainLayout->addWidget(timelinePanel);
	}

	// Create property editor
	auto entityPropertiesModel = std::make_shared<EntityPropertiesModel>(engineRoot->typeRegistry.get(), std::make_shared<ReflTypePropertyFactoryMap>(createSkyboltReflTypePropertyFactories(*engineRoot->typeRegistry)));
	
	PropertyEditorWidgetFactoryMapPtr factoryMap = createSkyboltEditorWidgetFactoryMap(DefaultEditorWidgetFactoryMapConfig{
		.listEditorIcons = createDefaultListEditorIcons()
		});
	auto* propertyEditor = new PropertyEditor(factoryMap, &mainWindow);
	{
		auto propertiesWidget = wrapWithVerticalScrollBar(propertyEditor);
		propertiesWidget->setMinimumWidth(300);

		auto* propertiesDock = new QDockWidget("Properties", &mainWindow);
		propertiesDock->setWidget(propertiesWidget);
		propertiesDock->setObjectName("Properties");
		propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		mainWindow.addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
	}

	auto scenarioPropertiesModel = std::make_shared<ScenarioPropertiesModel>(engineRoot->scenario.get());

	// Create entity creation toolbar
	EntityCreationToolBar* entityCreationToolbar;
	{
		auto poseEntityInFrontOfCamera = [world = &engineRoot->scenario->world, viewportCamera] (Entity& entity) {
			sim::EntityPtr camera = world->getEntityById(*viewportCamera);
			if (camera)
			{
				placeEntityOnPlanetInFrontOfCamera(*camera, entity);
			}
			};

		entityCreationToolbar = new EntityCreationToolBar(EntityCreationToolBarConfig{
			.world = &engineRoot->scenario->world,
			.entityFactory = engineRoot->entityFactory.get(),
			.onEntityCreated = poseEntityInFrontOfCamera,
			.parent = &mainWindow
			});
	}

	// Create scenario tree widget
	{
		auto scenarioTree = new ScenarioTreeWidget(ScenarioTreeWidgetConfig{
			.world = &engineRoot->scenario->world
			});

		auto scenarioPanelWidget = new QWidget(&mainWindow);
		QBoxLayout* layout = createBoxLayoutWithWidgets(std::vector<QWidget*>{entityCreationToolbar, scenarioTree}, scenarioPanelWidget, QBoxLayout::Direction::TopToBottom);

		auto* scenarioDock = new QDockWidget("Entities", &mainWindow);
		scenarioDock->setWidget(scenarioPanelWidget);
		scenarioDock->setObjectName("Entities");
		scenarioDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		mainWindow.addDockWidget(Qt::LeftDockWidgetArea, scenarioDock);

		QObject::connect(scenarioTree, &ScenarioTreeWidget::entitySelected, [entityPropertiesModel, world = &engineRoot->scenario->world, entityCreationToolbar, propertyEditor](const skybolt::sim::EntityId& entityId) {
			entityCreationToolbar->setSelectedEntity(entityId);
			EntityPtr entity = world->getEntityById(entityId);
			entityPropertiesModel->setEntity(entity.get());

			if (entityId != nullEntityId())
			{
				propertyEditor->setModel(entityPropertiesModel);
			}
			});

		QObject::connect(scenarioTree, &ScenarioTreeWidget::scenarioSelected, [scenarioPropertiesModel, scenarioTree, propertyEditor]() {
			propertyEditor->setModel(scenarioPropertiesModel);
			});
	}

	// Create entity name labels
	std::shared_ptr<VisNameLabels> visNameLabels;
	{
		auto hudGroup = engineRoot->scene->getBucketGroup(vis::Scene::Bucket::Hud);
		visNameLabels = std::make_shared<VisNameLabels>(&engineRoot->scenario->world, hudGroup, engineRoot->programs);
		auto simVisSystem = sim::findSystem<SimVisSystem>(*engineRoot->systemRegistry);
		simVisSystem->addBinding(visNameLabels);
	}

	// Create stats display system
	std::shared_ptr<skybolt::StatsDisplaySystem> statsDisplaySystem;
	{
		vis::Window* window = osgWindow->getWindow();
		statsDisplaySystem = std::make_shared<StatsDisplaySystem>(&visRoot->getViewer(), window->getView(), viewport->getFinalRenderTarget()->getOsgCamera());
		statsDisplaySystem->setVisible(false);
		engineRoot->systemRegistry->push_back(statsDisplaySystem);
	}

	ScenarioWorkspace scenarioWorkspace(engineRoot.get());
	QObject::connect(&scenarioWorkspace, &ScenarioWorkspace::scenarioFilenameChanged, &mainWindow, [&mainWindow] (const QString& filename) {
			mainWindow.setWindowTitle(filename);
		});

	// Create File menu
	{
		QMenu* fileMenu = mainWindow.menuBar()->addMenu("&File");

		std::shared_ptr<RecentFilesMenuPopulator> recentFileMenuPopulator;

		// New
		{
			auto action = new QAction("&New Scenario", &mainWindow);
			fileMenu->addAction(action);

			QObject::connect(action, &QAction::triggered, &mainWindow, [&scenarioWorkspace]() {
				scenarioWorkspace.newScenario();
			});
		}

		fileMenu->addSeparator();

		// Open
		{
			auto openAction = new QAction("&Open...", &mainWindow);
			fileMenu->addAction(openAction);

			auto openRecentMenu = fileMenu->addMenu("Open &Recent");

			recentFileMenuPopulator = std::make_shared<RecentFilesMenuPopulator>(*openRecentMenu, &settings,
				[&scenarioWorkspace](const QString& filename) {
					scenarioWorkspace.loadScenario(filename);
				});

			QObject::connect(openAction, &QAction::triggered, &mainWindow, [&mainWindow, recentFileMenuPopulator, &scenarioWorkspace]() {
				QString filename = QFileDialog::getOpenFileName(&mainWindow, "Open Scenario", QString(), "Scenario Files (*.scn);All Files (*)");
				if (!filename.isEmpty())
				{
					scenarioWorkspace.loadScenario(filename);
					recentFileMenuPopulator->addRecentFilename(filename);
				}
			});
		}

		fileMenu->addSeparator();

		// Save
		{
			auto saveAsFn = [&mainWindow, recentFileMenuPopulator, &scenarioWorkspace]() {
				QString filename = QFileDialog::getSaveFileName(&mainWindow, "Save Scenario", QString(), "Scenario Files (*.scn)");
				if (!filename.isEmpty())
				{
					scenarioWorkspace.saveScenario(filename);
					recentFileMenuPopulator->addRecentFilename(filename);
				}
			};

			auto saveAction = new QAction("&Save...", &mainWindow);
			fileMenu->addAction(saveAction);

			QObject::connect(saveAction, &QAction::triggered, &mainWindow, [saveAsFn, &scenarioWorkspace]() {
				if (scenarioWorkspace.getScenarioFilename().isEmpty())
				{
					saveAsFn();
				}
				else
				{
					scenarioWorkspace.saveScenario(scenarioWorkspace.getScenarioFilename());
				}
			});

			auto saveAsAction = new QAction("Save &As...", &mainWindow);
			fileMenu->addAction(saveAsAction);

			QObject::connect(saveAsAction, &QAction::triggered, &mainWindow, saveAsFn);
		}

		fileMenu->addSeparator();

		// Exit
		{
			QAction* exitAction = new QAction("E&xit", &mainWindow);
			fileMenu->addAction(exitAction);
			QObject::connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
		}
	}

	// Create View menu
	{
		QMenu* viewMenu = mainWindow.menuBar()->addMenu("&View");

		// Toggle show labels
		{
			QAction* action = new QAction("Show &Labels", &mainWindow);
			action->setCheckable(true);
			action->setChecked(true);
			viewMenu->addAction(action);

			QObject::connect(action, &QAction::toggled, &mainWindow, [visNameLabels](bool checked) {
				visNameLabels->setEntityVisibilityPredicate(checked ? &EntityVisibilityFilterable::visibilityOn : &EntityVisibilityFilterable::visibilityOff);
			});
		}

		// Toggle show stats
		{
			QAction* action = new QAction("Show &Stats", &mainWindow);
			action->setCheckable(true);
			action->setChecked(false);
			viewMenu->addAction(action);

			QObject::connect(action, &QAction::toggled, &mainWindow, [statsDisplaySystem](bool checked) {
				statsDisplaySystem->setVisible(checked);
			});
		}

		// Toggle viewport texture debug visualization
		{
			QAction* action = new QAction("Show Debug &Texture", &mainWindow);
			action->setCheckable(true);
			action->setChecked(false);
			viewMenu->addAction(action);

			QObject::connect(action, &QAction::toggled, &mainWindow, [
				renderOperationVisualization = osg::ref_ptr<skybolt::vis::RenderOperation>(),
				engineRoot = engineRoot.get(), viewport,
				window = osgWindow->getWindow()
			] (bool checked) mutable {
				if (checked)
				{
					if (!renderOperationVisualization)
					{
						renderOperationVisualization = vis::createRenderOperationVisualization(viewport, engineRoot->programs);
					}
					window->getRenderOperationSequence().addOperation(renderOperationVisualization);
				}
				else if (!checked && renderOperationVisualization)
				{
					window->getRenderOperationSequence().removeOperation(renderOperationVisualization);
				}
			});
		}

		// Toggle live shader editing
		{
			QAction* action = new QAction("Enable Live &Shader Editing", &mainWindow);
			action->setCheckable(true);
			action->setChecked(false);
			viewMenu->addAction(action);

			QObject::connect(action, &QAction::toggled, &mainWindow, [shaderSourceFileChangeMonitor = std::unique_ptr<skybolt::vis::ShaderSourceFileChangeMonitor>(), engineRoot = engineRoot.get()] (bool checked) mutable {
				shaderSourceFileChangeMonitor.reset();
				if (checked)
				{
					shaderSourceFileChangeMonitor = std::make_unique<vis::ShaderSourceFileChangeMonitor>(engineRoot->programs);
				}
			});
		}
	}

	// Create status bar
	mainWindow.setStatusBar(new QStatusBar(&mainWindow));
	mainWindow.statusBar()->addPermanentWidget(new LatestErrorWidget(&errorLogModel));

	// Handle command line arguments
	if (argc >= 2)
	{
		QString filename(argv[1]);
		scenarioWorkspace.loadScenario(filename);
	}
	else
	{
		scenarioWorkspace.newScenario();
	}

	// Start update loop
	createAndStartIntervalDtTimer(10, &mainWindow, [
		statusBar = mainWindow.statusBar(),
		simUpdater = &simUpdater,
		engineRoot = engineRoot.get(),
		entityPropertiesModel,
		visRoot,
		viewportInputSystem,
		osgWindow = osgWindow.get()
	] (sim::SecondsD wallDt){
		simUpdater->update(wallDt);
		entityPropertiesModel->update();
		visRoot->render();
		viewportInputSystem->setViewportHeight(osgWindow->height());
		statusBar->showMessage(getStatusBarText(*engineRoot));
	});

	splashScreen->finish(&mainWindow);

	mainWindow.show();

	splashScreen.reset();

	return app.exec();
}

int main(int argc, char** argv)
{
	try
	{
		file::Path executableDir = getExecutablePath();

		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
		QCoreApplication::setApplicationName("Skybolt Scenario Viewer");
		QCoreApplication::addLibraryPath(QString::fromStdString((executableDir / "qtPlugins").string()));

		return createAndExecuteApplication(argc, argv);
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