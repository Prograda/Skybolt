#include "CameraControllerWidget.h"
#include "MainWindow.h"
#include "ScenarioTreeWidget.h"

#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/Input/InputSystem.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/UpdateLoop/SimUpdater.h>
#include <SkyboltEngine/WindowUtil.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/Scenario/SimSnapshotRegistry.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngineQt/OsgWindow.h>
#include <SkyboltEngineQt/EntityPropertiesModel.h>
#include <SkyboltEngineQt/SkyboltQtPropertyReflection.h>
#include <SkyboltEngineQt/Icon/SkyboltIcons.h>
#include <SkyboltEngineQt/Input/InputPlatformQt.h>
#include <SkyboltEngineQt/Input/ViewportInputSystem.h>
#include <SkyboltEngineQt/ThirdParty/DarkTitleBar.h>
#include <SkyboltEngineQt/Style/DarkStyle.h>
#include <SkyboltWidgets/List/ListEditorWidget.h>
#include <SkyboltWidgets/CollapsiblePanel/CollapsiblePanelWidget.h>
#include <SkyboltWidgets/Property/PropertyEditor.h>
#include <SkyboltWidgets/Property/EditorWidgets.h>
#include <SkyboltWidgets/Property/DefaultEditorWidgets.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltWidgets/Property/QtPropertyReflectionConversion.h>
#include <SkyboltWidgets/Property/QtMetaTypes.h>
#include <SkyboltWidgets/ErrorLog/ErrorLogModel.h>
#include <SkyboltWidgets/ErrorLog/LatestErrorWidget.h>
#include <SkyboltWidgets/Timeline/TimeControlWidget.h>
#include <SkyboltWidgets/Timeline/TimelineWidget.h>
#include <SkyboltWidgets/Timeline/TimeRateDialog.h>
#include <SkyboltWidgets/Util/QtScrollAreaUtil.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>
#include <SkyboltWidgets/Util/RecentFilesMenuPopulator.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Spatial/LatLon.h>

#include <QApplication>
#include <QTreeView>
#include <QVBoxLayout>

#include <QDockWidget>
#include <QFileDialog>
#include <QScrollArea>
#include <QMenuBar>
#include <QAction>
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

static void createDefaultEntities(const EntityFactory& entityFactory, World& world)
{
	world.addEntity(entityFactory.createEntity("Stars"));
	world.addEntity(entityFactory.createEntity("SunBillboard"));
	world.addEntity(entityFactory.createEntity("MoonBillboard"));
}

static void loadScenario(const std::string& filename, const EngineRoot& engineRoot)
{
	auto entityFactory = [entityFactory = engineRoot.entityFactory.get()] (const std::string& templateName, const std::string& instanceName) {
		return entityFactory->createEntity(templateName, instanceName);
		};

	nlohmann::json json = readJsonFile(filename);
	engineRoot.scenario->world.removeAllEntities();
	createDefaultEntities(*engineRoot.entityFactory, engineRoot.scenario->world);
	readScenario(*engineRoot.typeRegistry, *engineRoot.scenario, entityFactory, json.at("scenario"));
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

static int createAndExecuteApplication(int argc, char** argv)
{
	// Create application
	ApplicationWithErrorHandling app(argc, argv);
	app.setStyle(new DarkStyle);

	// Create engine
	auto params = EngineCommandLineParser::parse(argc, argv);
	std::unique_ptr<EngineRoot> engineRoot = EngineRootFactory::create(params);

	SimUpdater simUpdater(engineRoot.get());

	// Allow user to reset the simulation state to t=0 if they rewind to the beginning
	std::shared_ptr<SimSnapshotRegistry> snapshotRegistry = createSnapshotRegistry(*engineRoot);
	useSnapshotsToResetSimulationStateAtTimelineStart(snapshotRegistry, engineRoot->scenario.get());

	// Build main window
	QSettings settings(QApplication::applicationName());
	MainWindow mainWindow(&settings);
	auto central = new QWidget(&mainWindow);
	mainWindow.setCentralWidget(central);
	mainWindow.setWindowTitle("Skybolt Scenario Viewer");
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
		//viewport->setCamera(getVisCamera(*simCamera));
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

	// Handle camera selection change event
	QObject::connect(cameraControllerWidget, &CameraControllerWidget::cameraSelectionChanged, [viewport, engineRoot = engineRoot.get(), viewportInputSystem](const sim::Entity* camera) {
		if (camera)
		{
			World* world = &engineRoot->scenario->world;
			viewport->setCamera(getVisCamera(*camera));
			auto simVisSystem = sim::findRequiredSystem<SimVisSystem>(*engineRoot->systemRegistry);
			simVisSystem->setSceneOriginProvider(SimVisSystem::sceneOriginFromEntity(world, camera->getId()));
			connectToCameraExclusivly(*viewportInputSystem, world, camera->getId());
		}
		});

	// Create time control panel
	{
		QWidget* timelinePanel = createTimeControlWidgetPanel(engineRoot.get(), &simUpdater, central);
		mainLayout->addWidget(timelinePanel);
	}

	// Create property editor
	EntitySelector entitySelector;
	auto entityPropertiesModel = std::make_shared<EntityPropertiesModel>(engineRoot->typeRegistry.get(), std::make_shared<ReflTypePropertyFactoryMap>(createSkyboltReflTypePropertyFactories(*engineRoot->typeRegistry)));
	{

		PropertyEditorWidgetFactoryMapPtr factoryMap = createDefaultEditorWidgetFactoryMap(DefaultEditorWidgetFactoryMapConfig{
			.listEditorIcons = createDefaultListEditorIcons()
			});
		auto* propertyEditor = new PropertyEditor(factoryMap, &mainWindow);
		propertyEditor->setModel(entityPropertiesModel);

		auto propertiesWidget = wrapWithVerticalScrollBar(propertyEditor);
		propertiesWidget->setMinimumWidth(300);

		auto* propertiesDock = new QDockWidget("Properties", &mainWindow);
		propertiesDock->setWidget(propertiesWidget);
		propertiesDock->setObjectName("Properties");
		propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		mainWindow.addDockWidget(Qt::RightDockWidgetArea, propertiesDock);


		entitySelector = [entityPropertiesModel, world = &engineRoot->scenario->world](const skybolt::sim::EntityId& entityId) {
			EntityPtr entity = world->getEntityById(entityId);
			entityPropertiesModel->setEntity(entity.get());
			};
	}

	// Create scenario tree widget
	{
		auto* scenarioTree = new ScenarioTreeWidget(ScenarioTreeWidgetConfig{
			.entitySelector = entitySelector,
			.world = &engineRoot->scenario->world
			});
		auto* scenarioDock = new QDockWidget("Entities", &mainWindow);
		scenarioDock->setWidget(scenarioTree);
		scenarioDock->setObjectName("Entities");
		scenarioDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		mainWindow.addDockWidget(Qt::LeftDockWidgetArea, scenarioDock);
	}

	// Create File menu
	{
		QMenu* fileMenu = mainWindow.menuBar()->addMenu("&File");

		QAction* openAction = new QAction("&Open...", &mainWindow);
		fileMenu->addAction(openAction);

		auto openRecentMenu = fileMenu->addMenu("Open &Recent");

		auto recentFileMenuPopulator = std::make_shared<RecentFilesMenuPopulator>(*openRecentMenu, &settings,
			[&mainWindow, engineRoot = engineRoot.get()](const QString& filename) {
				loadScenario(filename.toStdString(), *engineRoot);
			});

		QObject::connect(openAction, &QAction::triggered, &mainWindow, [&mainWindow, engineRoot = engineRoot.get(), recentFileMenuPopulator]() {
			QString filename = QFileDialog::getOpenFileName(&mainWindow, "Open Scenario", QString(), "Scenario Files (*.scn);All Files (*)");
			if (!filename.isEmpty())
			{
				loadScenario(filename.toStdString(), *engineRoot);
				recentFileMenuPopulator->addRecentFilename(filename);
			}
		});

		QAction* exitAction = new QAction("E&xit", &mainWindow);
		fileMenu->addAction(exitAction);
		QObject::connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
	}

	if (argc >= 2)
	{
		std::string filename(argv[1]);
		loadScenario(filename, *engineRoot);
	}

	// Start update loop
	createAndStartIntervalDtTimer(10, &mainWindow, [simUpdater = &simUpdater, entityPropertiesModel, visRoot, viewportInputSystem, osgWindow = osgWindow.get()] (sim::SecondsD wallDt){
		simUpdater->update(wallDt);
		entityPropertiesModel->update();
		visRoot->render();
		viewportInputSystem->setViewportHeight(osgWindow->height());
	});

	mainWindow.show();
	return app.exec();

}

int main(int argc, char** argv)
{
	try
	{
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