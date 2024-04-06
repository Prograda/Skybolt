/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "CaptureImageDailog.h"
#include "EditorPlugin.h"
#include "IconFactory.h"
#include "InputPlatformOis.h"
#include "QDialogHelpers.h"
#include "RecentFilesMenuPopulator.h"
#include "SprocketModel.h"
#include "SettingsEditor.h"
#include "TimeControlWidget.h"
#include "TimelineWidget.h"
#include "DataSeries/DataSeries.h"
#include "ContextAction/CreateContextActions.h"
#include "Entity/EntityCreatorWidget.h"
#include "Entity/EntityListModel.h"
#include "Entity/EntityPropertiesModel.h"
#include "Scenario/ScenarioPropertiesModel.h"
#include "Viewport/VisEntityIcons.h"
#include "Viewport/OsgWidget.h"

#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/EngineStats.h>
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltEngine/VisObjectsComponent.h>
#include <SkyboltEngine/Diagnostics/StatsDisplaySystem.h>
#include <SkyboltEngine/Input/LogicalAxis.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/ForcesVisBinding.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/SimVisBinding/VisNameLabels.h>
#include <SkyboltEngine/SimVisBinding/VisOrbits.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Camera.h>
#include <SkyboltVis/DisplaySettings.h>
#include <SkyboltVis/Shader/OsgShaderHelpers.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Renderable/Planet/Planet.h>
#include <SkyboltVis/Renderable/Arrows.h>
#include <SkyboltVis/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationUtil.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Shader/ShaderSourceFileChangeMonitor.h>
#include <SkyboltVis/Window/CaptureScreenshot.h>
#include <SkyboltVis/Window/Window.h>
#include <SkyboltCommon/File/OsDirectories.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/StringVector.h>
#include <SkyboltCommon/Range.h>

#include <px_sched/px_sched.h>

#include <filesystem>
#include <boost/log/trivial.hpp>

#include <ToolWindowManager/ToolWindowManagerArea.h>

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include <QToolBar>
#include <QVector3D>

#include <osgDB/Registry>

#include <algorithm>
#include <chrono>
#include <deque>
#include <sstream>

using namespace skybolt;
using namespace skybolt::sim;

class ViewportPropertiesModel : public PropertiesModel
{
public:
	ViewportPropertiesModel(vis::Scene* Scene, CameraComponent* camera) :
		mScene(Scene)
	{
		{
			mFov = createVariantProperty("Vertical FOV", 0.0);
			mFov->setValue(camera->getState().fovY * skybolt::math::radToDegF());
			mProperties.push_back(mFov);

			connect(mFov.get(), &VariantProperty::valueChanged, [=]()
			{
				camera->getState().fovY = mFov->value.toFloat() * skybolt::math::degToRadF();
			});
		}
		{
			mAmbientLight = createVariantProperty("Ambient Light", 0.0);
			mAmbientLight->setValue(mScene->getAmbientLightColor().x());
			mProperties.push_back(mAmbientLight);

			connect(mAmbientLight.get(), &VariantProperty::valueChanged, [=]()
			{
				float v = mAmbientLight->value.toFloat();
				mScene->setAmbientLightColor(osg::Vec3f(v, v, v));
			});
		}
	}

private:
	vis::Scene* mScene;
	VariantPropertyPtr mFov;
	VariantPropertyPtr mAmbientLight;
};

class PropertiesDialog : public QDialog
{
public:
	PropertiesDialog(vis::Scene* Scene, const sim::EntityPtr& camera, QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
	{
		setLayout(new QVBoxLayout);
		PropertyEditor* editor = new PropertyEditor({});
		layout()->addWidget(editor);
		editor->setModel(std::make_shared<ViewportPropertiesModel>(Scene, camera->getFirstComponent<CameraComponent>().get()));

		QPushButton* button = new QPushButton("Close");
		button->setAutoDefault(false);
		layout()->addWidget(button);
		connect(button, &QPushButton::pressed, this, &QDialog::accept);
	}
};

static const QString defaultWindowLayoutState = "AAAAAwAAADgAdABvAG8AbABXAGkAbgBkAG8AdwBNAGEAbgBhAGcAZQByAFMAdABhAHQAZQBGAG8AcgBtAGEAdAAAAAIAAAAAAQAAABYAbQBhAGkAbgBXAHIAYQBwAHAAZQByAAAACAAAAAACAAAAEABzAHAAbABpAHQAdABlAHIAAAAIAAAAAAMAAAAIAHQAeQBwAGUAAAAKAAAAABAAcwBwAGwAaQB0AHQAZQByAAAACgBzAHQAYQB0AGUAAAAKAAAAAFgAQQBBAEEAQQAvAHcAQQBBAEEAQQBFAEEAQQBBAEEAQwBBAEEAQQBBAC8AdwBBAEEAQgBuAGMAQQAvAC8ALwAvAC8AdwBFAEEAQQBBAEEAQgBBAEEAPQA9AAAACgBpAHQAZQBtAHMAAAAJAAAAAAIAAAAIAAAAAAMAAAAIAHQAeQBwAGUAAAAKAAAAAAgAYQByAGUAYQAAAA4AbwBiAGoAZQBjAHQAcwAAAAkAAAAAAgAAAAgAAAAAAgAAAAgAbgBhAG0AZQAAAAoAAAAAEABFAHgAcABsAG8AcgBlAHIAAAAIAGQAYQB0AGEAAAAAAQAAAAgAAAAAAgAAAAgAbgBhAG0AZQAAAAoAAAAAIgBFAG4AdABpAHQAeQAgAEMAbwBuAHQAcgBvAGwAbABlAHIAAAAIAGQAYQB0AGEAAAAAAQAAABgAYwB1AHIAcgBlAG4AdABJAG4AZABlAHgAAAACAAAAAAAAAAAIAAAAAAMAAAAIAHQAeQBwAGUAAAAKAAAAABAAcwBwAGwAaQB0AHQAZQByAAAACgBzAHQAYQB0AGUAAAAKAAAAAFgAQQBBAEEAQQAvAHcAQQBBAEEAQQBFAEEAQQBBAEEAQwBBAEEAQQBGAEwAUQBBAEEAQQBVAEEAQQAvAC8ALwAvAC8AdwBFAEEAQQBBAEEAQgBBAEEAPQA9AAAACgBpAHQAZQBtAHMAAAAJAAAAAAIAAAAIAAAAAAMAAAAIAHQAeQBwAGUAAAAKAAAAAAgAYQByAGUAYQAAAA4AbwBiAGoAZQBjAHQAcwAAAAkAAAAAAQAAAAgAAAAAAgAAAAgAbgBhAG0AZQAAAAoAAAAAEABWAGkAZQB3AHAAbwByAHQAAAAIAGQAYQB0AGEAAAAAAQAAABgAYwB1AHIAcgBlAG4AdABJAG4AZABlAHgAAAACAAAAAAAAAAAIAAAAAAMAAAAIAHQAeQBwAGUAAAAKAAAAAAgAYQByAGUAYQAAAA4AbwBiAGoAZQBjAHQAcwAAAAkAAAAAAQAAAAgAAAAAAgAAAAgAbgBhAG0AZQAAAAoAAAAAFABQAHIAbwBwAGUAcgB0AGkAZQBzAAAACABkAGEAdABhAAAAAAEAAAAYAGMAdQByAHIAZQBuAHQASQBuAGQAZQB4AAAAAgAAAAAAAAAAEABnAGUAbwBtAGUAdAByAHkAAAAKAAAAALAAQQBkAG4AUQB5AHcAQQBEAEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBBAEEAQgAzADgAQQBBAEEAUABNAEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBkAC8AQQBBAEEARAB6AEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBCADQAQQBBAEEAQQBBAEEAQQBBAEEAQQBBAEEAQQBBAEIAMwA4AEEAQQBBAFAATQAAAB4AZgBsAG8AYQB0AGkAbgBnAFcAaQBuAGQAbwB3AHMAAAAJAAAAAAA=";

class ViewportInput : public EventListener
{
public:
	ViewportInput(const InputPlatformPtr& inputPlatform) :
		mInputPlatform(inputPlatform)
	{
		mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);

		std::vector<InputDevicePtr> keyboards = inputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard);
		if (keyboards.empty())
		{
			throw std::runtime_error("Keyboard input device not found");
		}

		InputDevicePtr keyboard = keyboards.front();
		float rate = 1000;
		mForwardAxis = std::make_shared<KeyAxis>(keyboard, KC_S, KC_W, rate, rate, -1.0f, 1.0f);
		mLogicalAxes.push_back(mForwardAxis);

		mRightAxis = std::make_shared<KeyAxis>(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f);
		mLogicalAxes.push_back(mRightAxis);

		setEnabled(false);
	}

	~ViewportInput()
	{
		mInputPlatform->getEventEmitter()->removeEventListener(this);
	}

	void setEnabled(bool enabled)
	{
		for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeMouse))
		{
			device->setEnabled(enabled);
		}

		for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard))
		{
			device->setEnabled(enabled);
		}
		mEnabled = enabled;
	}

	void updateBeforeInput()
	{
		mInput = CameraController::Input::zero();
	}

	void updateAfterInput(float dt)
	{
		for (const LogicalAxisPtr& device : mLogicalAxes)
		{
			device->update(dt);
		}
		mInput.forwardSpeed = mForwardAxis->getState();
		mInput.rightSpeed = mRightAxis->getState();
		mInput.panSpeed /= dt;
		mInput.tiltSpeed /= dt;
		mInput.zoomSpeed /= dt;
		mInput.modifier1Pressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LSHIFT);
		mInput.modifier2Pressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LCONTROL);
	}

	CameraController::Input getInput() const
	{
		return mInput;
	}

	void onEvent(const Event &evt)
	{
		if (!mEnabled)
		{
			return;
		}

		if (const auto& event = dynamic_cast<const MouseEvent*>(&evt))
		{
			mInput.panSpeed += event->relState.x;
			mInput.tiltSpeed -= event->relState.y;
			mInput.zoomSpeed = event->relState.z;
		}
	}

private:
	InputPlatformPtr mInputPlatform;
	std::vector<LogicalAxisPtr> mLogicalAxes;
	LogicalAxisPtr mForwardAxis;
	LogicalAxisPtr mRightAxis;
	CameraController::Input mInput = CameraController::Input::zero();
	bool mEnabled = false;
};

template <typename Source, typename Result, typename Functor>
void mapContainer(const Source& c, Result& result, Functor &&f)
{
	std::transform(std::begin(c), std::end(c), std::inserter(result, std::end(result)), f);
}

QString settingsFilenameKey = "settingsFilename";

// Read the engine settings file if it already exists, otherwise prompts user to create one.
// The engine settings file is different to the QSettings, the former configures the engine
// while the latter configures UI defaults.
static nlohmann::json readOrCreateEngineSettingsFile(QWidget* parent, QSettings& settings)
{
	nlohmann::json result = createDefaultEngineSettings();
	QString settingsFilename = settings.value(settingsFilenameKey).toString();

	while (settingsFilename.isEmpty() || !QFile(settingsFilename).exists())
	{
		QMessageBox box;
		box.setWindowTitle("Settings");
		box.setText("Settings file required to run.\nClick Create to create a new file, or Load to load an existing file");
		QAbstractButton* createButton = box.addButton("Create...", QMessageBox::YesRole);
		QAbstractButton* loadButton = box.addButton("Load...", QMessageBox::YesRole);

		box.exec();
		if (box.clickedButton() == createButton)
		{
			auto appUserDataDir = file::getAppUserDataDirectory("Skybolt");
			settingsFilename = QString::fromStdString(appUserDataDir.append("Settings.json").string());

			settingsFilename = QFileDialog::getSaveFileName(parent, "Create Settings File", settingsFilename, "Json (*.json)");

			if (!settingsFilename.isEmpty())
			{
				// Create file
				settings.setValue(settingsFilenameKey, settingsFilename);
				std::filesystem::create_directories(file::Path(settingsFilename.toStdString()).parent_path());
				writeJsonFile(result, settingsFilename.toStdString());
				return result;
			}
		}
		else if (box.clickedButton() == loadButton)
		{
			settingsFilename = QFileDialog::getOpenFileName(parent, "Load Settings File", settingsFilename, "Json (*.json)");
		 	settings.setValue(settingsFilenameKey, settingsFilename);

			if (!settingsFilename.isEmpty())
			{
				break;
			}
		}
		else
		{
			throw std::runtime_error("Settings file not specified. Program will now exit.");
		}
	}

	// Load file
	result.update(readJsonFile(settingsFilename.toStdString()));
	return result;
}

static osg::ref_ptr<osg::Texture> createEntitySelectionIcon(int width = 32)
{
	int height = width;
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	std::uint8_t* p = image->data();
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			bool edge = (x == 0 || y == 0 || x == (width - 1) || y == (height - 1));
			*p++ = 255;
			*p++ = 255;
			*p++ = 255;
			*p++ = edge ? 255 : 0;
		}
	}

	return new osg::Texture2D(image);
}

MainWindow::MainWindow(const std::vector<PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& editorPluginFactories, QWidget *parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags),
	ui(new Ui::MainWindow),
	mSettings(QApplication::applicationName())
{
	RecentFilesMenuPopulator::FileOpener fileOpener = [this](const QString& filename) {
		open(filename);
	};

	ui->setupUi(this);
	ui->actionNewEntityTemplate->setEnabled(false); // Entity Template editing is not implemented
	mViewMenuToolWindowSeparator = ui->menuView->addSeparator();
	mRecentFilesMenuPopulator.reset(new RecentFilesMenuPopulator(ui->menuRecentFiles, &mSettings, fileOpener));

	mEngineSettings = readOrCreateEngineSettingsFile(this, mSettings);

	mEngineRoot = EngineRootFactory::create(enginePluginFactories, mEngineSettings);
	mSimStepper = std::make_unique<SimStepper>(mEngineRoot->systemRegistry);

	mOsgWidget = new OsgWidget(getDisplaySettingsFromEngineSettings(mEngineSettings), this);

	mViewport = new vis::DefaultRenderCameraViewport([&]{
		vis::DefaultRenderCameraViewportConfig c;
		c.scene = mEngineRoot->scene;
		c.programs = &mEngineRoot->programs;
		c.shadowParams = getShadowParams(mEngineRoot->engineSettings);
		c.cloudRenderingParams = getCloudRenderingParams(mEngineRoot->engineSettings);
		return c;
	}());
	mOsgWidget->getWindow()->getRenderOperationSequence().addOperation(mViewport);

	mOsgWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mOsgWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

	mStatsDisplaySystem = std::make_shared<StatsDisplaySystem>(&mOsgWidget->getVisRoot()->getViewer(), mOsgWidget->getWindow()->getView(), mViewport->getFinalRenderTarget()->getOsgCamera());
	mStatsDisplaySystem->setVisible(false);
	mEngineRoot->systemRegistry->push_back(mStatsDisplaySystem);

	mInputPlatform.reset(new InputPlatformOis(std::to_string(size_t(winId())), 800, 600)); // TODO: use actual resolution
	mViewportInput.reset(new ViewportInput(mInputPlatform));

	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);

	mSprocketModel.reset(new SprocketModel(mEngineRoot.get(), mInputPlatform.get()));

	setProjectFilename("");
	
	// Connect menus
	QObject::connect(ui->actionNewScenario, SIGNAL(triggered()), this, SLOT(newScenario()));
	QObject::connect(ui->actionNewEntityTemplate, SIGNAL(triggered()), this, SLOT(newEntityTemplate()));
	QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	QObject::connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exit()));
	QObject::connect(ui->actionShowViewportStats, &QAction::triggered, this, [this](bool visible) {mStatsDisplaySystem->setVisible(visible); });
	QObject::connect(ui->actionShowViewportTextures, SIGNAL(triggered(bool)), this, SLOT(setViewportTextureDisplayEnabled(bool)));
	QObject::connect(ui->actionCaptureImage, SIGNAL(triggered()), this, SLOT(captureImage()));
	QObject::connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(editEngineSettings()));
	QObject::connect(ui->actionLiveShaderEditing, SIGNAL(triggered(bool)), this, SLOT(setLiveShaderEditingEnabled(bool)));
	
	World* world = mEngineRoot->simWorld.get();

	mSceneObjectPicker = createSceneObjectPicker(world);

	Scenario& scenario = mEngineRoot->scenario;

	auto hudGroup = mEngineRoot->scene->getBucketGroup(vis::Scene::Bucket::Hud);

	mVisSelectedEntityIcon = new VisEntityIcons(mEngineRoot->programs, createEntitySelectionIcon());
	hudGroup->addChild(mVisSelectedEntityIcon);

	mVisNameLabels = std::make_unique<VisNameLabels>(world, hudGroup, mEngineRoot->programs);

	osg::ref_ptr<osg::Program> unlitColoredProgram = mEngineRoot->programs.getRequiredProgram("unlitColored");

	{
		vis::Polyline::Params params;
		params.program = unlitColoredProgram;
		mVisOrbits.reset(new VisOrbits(world, hudGroup, params, mEngineRoot->julianDateProvider));
	}
	
	{
		vis::Arrows::Params params;
		params.program = unlitColoredProgram;

		auto arrows = std::make_shared<vis::Arrows>(params);
		mEngineRoot->scene->addObject(arrows);
		mForcesVisBinding.reset(new ForcesVisBinding(world, arrows));
	}

	mToolWindowManager = new ToolWindowManager(this);
	// setup tool window manager
	connect(mToolWindowManager, SIGNAL(toolWindowVisibilityChanged(QWidget*, bool)), this, SLOT(toolWindowVisibilityChanged(QWidget*, bool)));

	setCentralWidget(mToolWindowManager);

	setStatusBar(new QStatusBar(this));

	// Create center layout
	QWidget* viewWidget;
	{
		QBoxLayout* layout = new QVBoxLayout();
		layout->setMargin(0);
		viewWidget = new QWidget();
		viewWidget->setLayout(layout);

		QToolBar* toolbar = createViewportToolBar();
		layout->addWidget(toolbar);

		layout->addWidget(mOsgWidget);

		{
			TimelineWidget* timeline = new TimelineWidget;
			layout->addWidget(timeline);
			TimeControlWidget* timeControl = new TimeControlWidget;
			layout->addWidget(timeControl);

			timeline->setTimeSource(&scenario.timeSource);

			scenario.timeSource.timeChanged.connect([=](double time)
			{
				timeline->setBufferedRange(TimeRange(0, time));
			});

			timeControl->setTimeSource(&scenario.timeSource);
		}
	}

	addToolWindow("Viewport", viewWidget);

	float timeOfDayInHours = 20;
	scenario.startJulianDate = calcJulianDate(2017, 8, 17, timeOfDayInHours);

	// Load editor plugins
	{
		EditorPluginConfig config;
		config.uiController = std::make_shared<UiController>();
		config.uiController->propertiesModelSetter = [this](const PropertiesModelPtr& model) {
			if (mPropertiesEditor)
				mPropertiesEditor->setModel(nullptr);
			mPropertiesModel.reset();

			if (mPropertiesEditor)
				mPropertiesEditor->setModel(model);
			mPropertiesModel = model;
		};
		config.uiController->toolWindowRaiser = [this](QWidget* widget) {
			raiseToolWindow(widget);
		};
		config.engineRoot = mEngineRoot.get();
		config.fileLocator = mSprocketModel->fileLocator;
		config.inputPlatform = mInputPlatform;
		config.dataSeriesRegistry = std::make_shared<DataSeriesRegistry>();

		mapContainer(editorPluginFactories, mPlugins, [config](const EditorPluginFactory& factory) {
			return factory(config);
		});
	}

	{
		PropertyEditorWidgetFactoryMap factories;
		for (const EditorPluginPtr& plugin : mPlugins)
		{
			auto items = plugin->getPropertyEditorWidgetFactories();
			factories.insert(items.begin(), items.end());
		}

		mPropertiesEditor = new PropertyEditor(factories, this);
		mPropertiesEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		addToolWindow("Properties", mPropertiesEditor);
	}

	mContextActions = createContextActions(*mEngineRoot);

	{
		std::vector<TreeItemType> itemTypes;

		for (const EditorPluginPtr& plugin : mPlugins)
		{
			auto items = plugin->getTreeItemTypes();
			itemTypes.insert(itemTypes.end(), items.begin(), items.end());
		}

		WorldTreeWidgetConfig config;
		config.world = world;
		config.factory = mEngineRoot->entityFactory.get();
		config.itemTypes = itemTypes;
		config.scenario = &scenario;
		config.contextActions = mContextActions;

		mWorldTreeWidget = new WorldTreeWidget(config);
		QObject::connect(mWorldTreeWidget, &WorldTreeWidget::selectionChanged, this, &MainWindow::explorerSelectionChanged);

		addToolWindow("Explorer", mWorldTreeWidget);
	}
	for (const EditorPluginPtr& plugin : mPlugins)
	{
		auto windows = plugin->getToolWindows();
		for (const auto& window : windows)
		{
			addToolWindow(window.name, window.widget);
		}
	}

	addViewportMenuActions();

	connect(mOsgWidget, &OsgWidget::mouseDown, this, [this](Qt::MouseButton button, const QPointF& position) {
		onViewportMouseDown(button, position);
	});

	clearProject();

	// Update immediately to sync ths vis to the sim.
	// Necessary because Qt may decide to render the viewport before the update timer fires for the first time.
	update();

	QTimer::singleShot(0, this, SLOT(updateIfIntervalElapsed()));
}

MainWindow::~MainWindow()
{
	mForcesVisBinding.reset();
	mVisNameLabels.reset();
	mVisOrbits.reset();

	mPlugins.clear();

	delete mToolWindowManager;

	mSprocketModel.reset();
	mCurrentSimCamera.reset();
	mSimStepper.reset();
	mEngineRoot.reset();
}

double minFrameDuration = 0.01;

double latestSimTime = 0;

static void simulate(SimStepper& simStepper, TimeSource& timeSource, float dt)
{
	if (timeSource.getTime() == 0)
	{
		latestSimTime = timeSource.getTime();
	}

	timeSource.update(dt);
	double dtSim = std::max(0.0, timeSource.getTime() - latestSimTime);
	if (dtSim > 0.0)
	{
		latestSimTime = timeSource.getTime();
	}
	System::StepArgs args;
	args.dtSim = dtSim;
	args.dtWallClock = dt;
	simStepper.step(args);
}

static QString addSeparator(const QString& str)
{
	QString result = str;
	if (!str.isEmpty())
	{
		result += ", ";
	}
	return result;
}

void MainWindow::update()
{
	double dt = double(mUpdateTimer.count<std::chrono::milliseconds>()) / 1000.0;
	mUpdateTimer.reset();
	mUpdateTimer.start();

	if (mInputPlatform) // TODO
	{
		if (mDisableInputSystemOnNextUpdate)
		{
			mViewportInput->setEnabled(false);
			mDisableInputSystemOnNextUpdate = false;
		}
		mViewportInput->updateBeforeInput();
		mInputPlatform->update();
		mViewportInput->updateAfterInput(dt);
	}

	EngineRoot* engineRoot = mSprocketModel->engineRoot;

	engineRoot->scenario.timeSource.update(dt);

	if (mShaderSourceFileChangeMonitor)
	{
		mShaderSourceFileChangeMonitor->update();
	}

	auto simVisSystem = findSystem<SimVisSystem>(*engineRoot->systemRegistry);
	assert(simVisSystem);
	const GeocentricToNedConverter& coordinateConverter = simVisSystem->getCoordinateConverter();

	if (mCurrentSimCamera)
	{
		simVisSystem->setSceneOriginProvider(SimVisSystem::sceneOriginFromEntity(mCurrentSimCamera));

		if (auto controller = mCurrentSimCamera->getFirstComponent<CameraControllerComponent>())
		{
			controller->cameraController->setInput(mViewportInput->getInput());
		}
	}

	simulate(*mSimStepper, engineRoot->scenario.timeSource, dt);

	if (mCurrentSimCamera)
	{
		mVisSelectedEntityIcon->syncVis(coordinateConverter);
		mVisNameLabels->syncVis(coordinateConverter);
		mVisOrbits->syncVis(coordinateConverter);
		mForcesVisBinding->syncVis(coordinateConverter);
	}

	if (Updatable* updatablePropertiesModel = dynamic_cast<Updatable*>(mPropertiesModel.get()))
	{
		updatablePropertiesModel->update();
	}

	mOsgWidget->update(); // TODO only redraw if something changed

	mWorldTreeWidget->update();

	QString status;
	if (engineRoot->stats.terrainTileLoadQueueSize)
	{
		status += "Loading terrain tiles: " + QString::number(engineRoot->stats.terrainTileLoadQueueSize);
	}

	if (engineRoot->stats.featureTileLoadQueueSize)
	{
		status = addSeparator(status);
		status += "Loading feature tiles: " + QString::number(engineRoot->stats.featureTileLoadQueueSize);
	}

	uint32_t activeTasks = engineRoot->scheduler->active_threads();
	if (activeTasks)
	{
		status = addSeparator(status);
		status += "Processing tasks: " + QString::number(activeTasks);
	}
	statusBar()->showMessage(status);
}

void MainWindow::updateIfIntervalElapsed()
{
	QTimer::singleShot(0, this, SLOT(updateIfIntervalElapsed()));

	double elapsed = double(mUpdateTimer.count<std::chrono::milliseconds>()) / 1000.0;
	if (elapsed < minFrameDuration)
	{
		return;
	}

	update();
}

static bool isPlanet(const Entity& entity)
{
	return getPosition(entity).has_value() && entity.getFirstComponent<PlanetComponent>() != nullptr;
}

static bool entityPredicateAlwaysFalse(const Entity& entity)
{
	return false;
}

static sim::CameraControllerSelectorPtr getCameraControllerSelector(const sim::Entity& entity)
{	
	if (auto controller = entity.getFirstComponent<sim::CameraControllerComponent>())
	{
		return std::dynamic_pointer_cast<sim::CameraControllerSelector>(controller->cameraController);
	}
	return nullptr;
}

class CameraControllerWidget : public QWidget
{
public:
	CameraControllerWidget(sim::World* world, QWidget* parent = nullptr) :
		mWorld(world),
		QWidget(parent)
	{
		setLayout(new QHBoxLayout());
		mCameraModeCombo = new QComboBox();
		mCameraModeCombo->setToolTip("Camera Mode");
		mCameraModeCombo->setEnabled(false);
		layout()->addWidget(mCameraModeCombo);

		mTargetListModel = new EntityListModel(world, &entityPredicateAlwaysFalse);
		auto proxyModel = new QSortFilterProxyModel(this);
		proxyModel->setSourceModel(mTargetListModel);
		

		mCameraTargetCombo = new QComboBox();
		mCameraTargetCombo->setToolTip("Camera Target");
		mCameraTargetCombo->setModel(proxyModel);
		mCameraTargetCombo->setEnabled(false);
		layout()->addWidget(mCameraTargetCombo);
	}

	void setCamera(const sim::EntityPtr& camera)
	{
		mCamera = camera;
		sim::CameraControllerSelectorPtr cameraControllerSelector = camera ? getCameraControllerSelector(*camera) : nullptr;

		// Clear
		mCameraModeCombo->disconnect();
		mCameraTargetCombo->disconnect();
		mCameraModeCombo->clear();
		mControllerConnections.clear();

		if (!cameraControllerSelector)
		{
			mCameraModeCombo->setEnabled(false);
			mCameraTargetCombo->setEnabled(false);
			mTargetListModel->setEntityFilter(&entityPredicateAlwaysFalse);
			return;
		}

		mCameraModeCombo->setEnabled(true);
		mCameraTargetCombo->setEnabled(true);

		// Mode combo
		for (const auto& item : cameraControllerSelector->getControllers())
		{
			mCameraModeCombo->addItem(QString::fromStdString(item.first));
		}

		const std::string& currentControllerName = cameraControllerSelector->getSelectedControllerName();
		mCameraModeCombo->setCurrentText(QString::fromStdString(currentControllerName));
		updateTargetFilterForControllerName(currentControllerName);

		connect(mCameraModeCombo, &QComboBox::currentTextChanged, [=](const QString& text)
		{
			cameraControllerSelector->selectController(text.toStdString());
		});

		mControllerConnections.push_back(cameraControllerSelector->controllerSelected.connect([this](const std::string& name) {
			mCameraModeCombo->blockSignals(true);
			mCameraModeCombo->setCurrentText(QString::fromStdString(name));
			mCameraModeCombo->blockSignals(false);

			updateTargetFilterForControllerName(name);
		}));

		// Target combo
		if (auto target = cameraControllerSelector->getTarget())
		{
			mCameraTargetCombo->setCurrentText(QString::fromStdString(getName(*target)));
		}
		
		connect(mCameraTargetCombo, &QComboBox::currentTextChanged, [=](const QString& text)
		{
			EntityPtr object = sim::findObjectByName(*mWorld, text.toStdString());
			if (object)
			{
				cameraControllerSelector->setTarget(object.get());
			}
		});

		mControllerConnections.push_back(cameraControllerSelector->targetChanged.connect([this](Entity* target) {
			mCameraTargetCombo->blockSignals(true);
			mCameraTargetCombo->setCurrentText(target ? QString::fromStdString(getName(*target)) : "");
			mCameraTargetCombo->blockSignals(false);
		}));
	}

private:
	void updateTargetFilterForControllerName(const std::string& name)
	{
		if (name == "Globe")
		{
			mTargetListModel->setEntityFilter(isPlanet);
		}
		else if (mCamera)
		{
			mTargetListModel->setEntityFilter([cameraName = getName(*mCamera)] (const sim::Entity& entity) {
				return getPosition(entity).has_value()
					&& entity.getFirstComponent<TemplateNameComponent>() != nullptr
					&& getName(entity) != cameraName;
			});
		}
		else
		{
			mTargetListModel->setEntityFilter(&entityPredicateAlwaysFalse);
		}
	}

private:
	sim::World* mWorld;
	sim::EntityPtr mCamera;
	QComboBox* mCameraModeCombo;
	QComboBox* mCameraTargetCombo;
	EntityListModel* mTargetListModel;
	std::vector<boost::signals2::scoped_connection> mControllerConnections;
};

QToolBar* MainWindow::createViewportToolBar()
{
	QToolBar* toolbar = new QToolBar();

	World* world = mEngineRoot->simWorld.get();

	{
		mCameraCombo = new QComboBox();
		mCameraCombo->setToolTip("Camera");
		mCameraCombo->setModel(new EntityListModel(world, [] (const Entity& entity) {
			return entity.getFirstComponent<CameraComponent>() != nullptr;
		}));

		connect(mCameraCombo, &QComboBox::currentTextChanged, [=](const QString& text)
		{
			EntityPtr object = findObjectByName(*world, text.toStdString());
			setCamera(object);
		});

		toolbar->addWidget(mCameraCombo);

		mCameraControllerWidget = new CameraControllerWidget(world);
		toolbar->addWidget(mCameraControllerWidget);
	}

	QIcon icon = getDefaultIconFactory().createIcon(IconFactory::Icon::Settings);
	vis::ScenePtr Scene = mEngineRoot->scene;
	toolbar->addAction(icon, "Settings", [=] {
		if (mCurrentSimCamera)
		{
			PropertiesDialog dialog(Scene.get(), mCurrentSimCamera, this);
			dialog.exec();
		}
	});

	return toolbar;
}

static QVariantMap toVariantMap(const QByteArray& array)
{
	QVariantMap variantMap;
	QDataStream stream(const_cast<QByteArray*>(&array), QIODevice::OpenMode::enum_type::ReadOnly);
	stream >> variantMap;
	return variantMap;
}

#define FOREACH_CALL(container, fn, ...) \
for (const auto& __item__ : container) \
	__item__->fn(__VA_ARGS__);

void MainWindow::clearProject()
{
	FOREACH_CALL(mPlugins, clearProject);

	setProjectFilename("");

	mSprocketModel->engineRoot->simWorld->removeAllEntities();

	mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(defaultWindowLayoutState.toUtf8())));
}

void createDefaultObjects(World& world, const EntityFactory& factory)
{
	world.addEntity(factory.createEntity("Stars"));
	world.addEntity(factory.createEntity("SunBillboard"));
	world.addEntity(factory.createEntity("MoonBillboard"));
}

void MainWindow::newScenario()
{
	const EntityFactory& factory = *mSprocketModel->engineRoot->entityFactory;
	World& simWorld = *mSprocketModel->engineRoot->simWorld;

	clearProject();

	// Create camera
	sim::EntityPtr camera = factory.createEntity("Camera");
	simWorld.addEntity(camera);
	setCamera(camera);

	createDefaultObjects(simWorld, factory);

	// Create earth
	auto planet = factory.createEntity("PlanetEarth");
	simWorld.addEntity(planet);
	setCameraTarget(planet.get());
}

void MainWindow::newEntityTemplate()
{
	clearProject();
}

QString projectFileExtension = ".proj";
QString projectFileFilter = "Project Files (*" + projectFileExtension + ")";

QString defaultDirKey = "defaultDir";

QString MainWindow::getDefaultProjectDirectory() const
{
	QString defaultDir = mSettings.value(defaultDirKey).toString();
	if (defaultDir.isEmpty())
	{
		defaultDir = QDir::homePath();
	}
	return defaultDir;
}

void MainWindow::open()
{
	QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Project"),
		getDefaultProjectDirectory(), projectFileFilter);

	if (!filename.isEmpty())
	{
		QDir currentDir;
		mSettings.setValue(defaultDirKey, currentDir.absoluteFilePath(filename));

		open(filename);
	}
}

static QByteArray toByteArray(const QVariantMap& variant)
{
	QByteArray array;
	QDataStream stream(&array, QIODevice::OpenMode::enum_type::WriteOnly);
	stream << variant;
	return array;
}

void MainWindow::open(const QString& filename)
{
	if (!QFileInfo::exists(filename))
		return;

	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly))
		return;

	EngineRoot* engineRoot = mSprocketModel->engineRoot;

	// TODO: prompt to save changes
	clearProject();
	createDefaultObjects(*engineRoot->simWorld, *engineRoot->entityFactory);

	setProjectFilename(filename);

	try
	{
		QByteArray wholeFile = file.readAll();
		QJsonObject const json = QJsonDocument::fromJson(wholeFile).object();

		loadProject(json);
		mRecentFilesMenuPopulator->addFilename(filename);
	}
	catch (std::exception &e)
	{
		QMessageBox::critical(this, "Error", e.what());
		clearProject();
	}
}

static nlohmann::json toNlohmannJson(const QJsonObject& json)
{
	QJsonDocument doc(json);
	QString str(doc.toJson(QJsonDocument::Compact));
	return nlohmann::json::parse(str.toStdString());
}

static QJsonObject toQJson(const nlohmann::json& json)
{
	auto doc = QJsonDocument::fromJson(QString::fromStdString(json.dump()).toUtf8());
	return doc.object();
}

void MainWindow::loadProject(const QJsonObject& json)
{
	QJsonValue value = json["scenario"];
	if (!value.isUndefined())
	{
		loadScenario(mEngineRoot->scenario, toNlohmannJson(value.toObject()));
	}

	value = json["geometry"];
	if (!value.isUndefined())
	{
		mToolWindowManager->restoreGeometry(QByteArray::fromBase64(value.toString().toUtf8()));
	}

	value = json["windows"];
	if (!value.isUndefined())
	{
		mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(value.toString().toUtf8())));
	}

	value = json["entities"];
	if (!value.isUndefined())
	{
		loadEntities(*mEngineRoot->simWorld, *mEngineRoot->entityFactory, toNlohmannJson(value.toObject()));
	}

	value = json["viewport"];
	if (!value.isUndefined())
	{
		loadViewport(value.toObject());
	}

	FOREACH_CALL(mPlugins, loadProject, json);
}

void MainWindow::save(QFile& file)
{
	QJsonObject json;
	saveProject(json);

	file.write(QJsonDocument(json).toJson());
}

void MainWindow::saveProject(QJsonObject& json) const
{
	json["scenario"] = toQJson(saveScenario(mSprocketModel->engineRoot->scenario));
	json["windows"] = QString(toByteArray(mToolWindowManager->saveState()).toBase64());
	json["geometry"] = QString(saveGeometry().toBase64());
	json["entities"] = toQJson(saveEntities(*mSprocketModel->engineRoot->simWorld));
	json["viewport"] = saveViewport();

	FOREACH_CALL(mPlugins, saveProject, json);
}

void MainWindow::save()
{
	if (mProjectFilename.isEmpty())
	{
		saveAs();
	}
	else
	{
		QFile file(mProjectFilename);
		if (file.open(QIODevice::WriteOnly))
		{
			save(file);
		}
	}
}

void MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Project"),
		getDefaultProjectDirectory(), projectFileFilter);

	if (!fileName.isEmpty())
	{
		if (!fileName.endsWith(projectFileExtension, Qt::CaseInsensitive))
			fileName += projectFileExtension;

		QFile file(fileName);
		if (file.open(QIODevice::WriteOnly))
		{
			save(file);
		}
		setProjectFilename(fileName);

		QDir currentDir;
		mSettings.setValue(defaultDirKey, currentDir.absoluteFilePath(fileName));
	}
}

static osg::Vec3 readJsonOsgVec3(const QJsonArray& v)
{
	if (v.size() != 3)
	{
		throw std::runtime_error("Incorrect number of elements in vector. Expected 3.");
	}
	return osg::Vec3(v[0].toDouble(), v[1].toDouble(), v[2].toDouble());
}

void MainWindow::loadViewport(const QJsonObject& json)
{
	{
		auto it = json.find("camera");
		if (it != json.end())
		{
			std::string name = it.value().toString().toStdString();
			EntityPtr camera = findObjectByName(*mEngineRoot->simWorld, name);
			setCamera(camera);
		}
	}
	{
		auto it = json.find("ambientLight");
		if (it != json.end())
		{
			mEngineRoot->scene->setAmbientLightColor(readJsonOsgVec3(it.value().toArray()));
		}
	}
}

static QJsonArray writeJson(const osg::Vec3& v)
{
	return { v.x(), v.y(), v.z() };
}

QJsonObject MainWindow::saveViewport() const
{
	QJsonObject json;
	if (mCurrentSimCamera)
	{
		json["camera"] = QString::fromStdString(getName(*mCurrentSimCamera));
		json["ambientLight"] = writeJson(mEngineRoot->scene->getAmbientLightColor());
	}
	return json;
}

static vis::CameraPtr getFirstVisCamera(const sim::Entity& simCamera)
{
	auto bindings = simCamera.getFirstComponent<SimVisBindingsComponent>();
	if (bindings)
	{
		for (const auto& binding : bindings->bindings)
		{
			if (auto cameraBinding = dynamic_cast<CameraSimVisBinding*>(binding.get()))
			{
				return cameraBinding->getCamera();
			}
		}
	}
	return nullptr;
}

void MainWindow::setCamera(const sim::EntityPtr& simCamera)
{
	if (mCurrentSimCamera != simCamera)
	{
		mCurrentSimCamera = simCamera;
		mCameraControllerWidget->setCamera(mCurrentSimCamera);

		vis::CameraPtr visCamera = mCurrentSimCamera ? getFirstVisCamera(*mCurrentSimCamera) : nullptr;
		mViewport->setCamera(visCamera);

		mCameraCombo->setCurrentText(simCamera ? QString::fromStdString(getName(*simCamera)) : "");
	}
}

void MainWindow::setCameraTarget(Entity* target)
{
	if (mCurrentSimCamera)
	{
		vis::CameraPtr visCamera = getFirstVisCamera(*mCurrentSimCamera);
		if (auto controller = mCurrentSimCamera->getFirstComponent<CameraControllerComponent>())
		{
			controller->cameraController->setTarget(target);
		}
	}
}

void MainWindow::about()
{
	QMessageBox::about(this, QApplication::applicationName(), "Copyright 2019 Matthew Paul Reid"); // TODO
}

void MainWindow::exit()
{
	close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	event->ignore();
	QMainWindow::closeEvent(event);
}

void MainWindow::captureImage()
{
	mOsgWidget->setFixedWidth(1920);
	mOsgWidget->setFixedHeight(1080);

	QString defaultSequenceName = "untitled";
	if (!mProjectFilename.isEmpty())
	{
		defaultSequenceName = QString::fromStdString(std::filesystem::path(mProjectFilename.toStdString()).stem().string());
	}

	showCaptureImageSequenceDialog([=](double time, const QString& filename) {
			mEngineRoot->scenario.timeSource.setTime(time);
			update();
			bool fullyLoadEachFrameBeforeProgressing = false;
			if (fullyLoadEachFrameBeforeProgressing)
			{
				while (mEngineRoot->stats.terrainTileLoadQueueSize > 0)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(1ms);
					updateIfIntervalElapsed();
				}
			}
			QImage image = mOsgWidget->grabFramebuffer();
			image.save(filename);
		}, defaultSequenceName, this);

	mOsgWidget->setMinimumSize(1, 1);
	mOsgWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void MainWindow::editEngineSettings()
{
	QString settingsFilename = mSettings.value(settingsFilenameKey).toString();

	SettingsEditor editor(settingsFilename, mEngineSettings, this);
	auto dialog = createDialog(&editor, "Settings");
	dialog->setMinimumWidth(500);
	dialog->setMinimumHeight(500);

	if (dialog->exec() == QDialog::Accepted)
	{
		settingsFilename = editor.getSettingsFilename();
		mSettings.setValue(settingsFilenameKey, settingsFilename);
		writeJsonFile(editor.getJson(), settingsFilename.toStdString());
	}
}

void MainWindow::setViewportTextureDisplayEnabled(bool enabled)
{
	if (enabled && !mRenderOperationVisualization)
	{
		mRenderOperationVisualization = vis::createRenderOperationVisualization(mViewport, mEngineRoot->programs);
		mOsgWidget->getWindow()->getRenderOperationSequence().addOperation(mRenderOperationVisualization);
	}
	else if (!enabled && mRenderOperationVisualization)
	{
		mOsgWidget->getWindow()->getRenderOperationSequence().removeOperation(mRenderOperationVisualization);
	}
}

void MainWindow::setLiveShaderEditingEnabled(bool enabled)
{
	mShaderSourceFileChangeMonitor.reset();
	if (enabled)
	{
		mShaderSourceFileChangeMonitor = std::make_unique<vis::ShaderSourceFileChangeMonitor>(mEngineRoot->programs);
	}
}

void MainWindow::setPropertiesModel(PropertiesModelPtr properties)
{
	mPropertiesEditor->setModel(nullptr);
	mPropertiesModel = std::move(properties);
	mPropertiesEditor->setModel(mPropertiesModel);
}

void MainWindow::setSelectedEntity(skybolt::sim::EntityId entityId)
{
	mSelectedEntityId = entityId;
	std::set<sim::Entity*> selection;
	auto entity = mEngineRoot->simWorld->getEntityById(entityId);
	if (entity)
	{
		selection.insert(entity);
	}
	mVisSelectedEntityIcon->setEntities(selection);
	
	mWorldTreeWidget->blockSignals(true); // prevent tree widget signaling explorerSelectionChanged
	mWorldTreeWidget->setSelectedEntity(entity);
	mWorldTreeWidget->blockSignals(false);
}

void MainWindow::explorerSelectionChanged(const TreeItem& item)
{
	setSelectedEntity(nullEntityId());

	if (auto entityItem = dynamic_cast<const EntityTreeItem*>(&item))
	{
		setSelectedEntity(entityItem->data);
		setPropertiesModel(std::make_shared<EntityPropertiesModel>( mEngineRoot->simWorld->getEntityById(mSelectedEntityId)));
	}
	else if (auto scenarioItem = dynamic_cast<const ScenarioTreeItem*>(&item))
	{
		setPropertiesModel(std::make_shared<ScenarioPropertiesModel>(&mSprocketModel->engineRoot->scenario));
	}

	FOREACH_CALL(mPlugins, explorerSelectionChanged, item);
}

std::optional<PickedSceneObject> MainWindow::pickSceneObjectAtPointInWindow(const QPointF& position, const EntitySelectionPredicate& predicate) const
{
	glm::vec2 pointNdc = glm::vec2(float(position.x()) / mOsgWidget->width(), float(position.y()) / mOsgWidget->height());
	glm::dmat4 transform = calcCurrentViewProjTransform();
	return mSceneObjectPicker(transform, pointNdc, 0.04, predicate);
}

std::optional<sim::Vector3> MainWindow::pickPointOnPlanetAtPointInWindow(const QPointF& position) const
{
   glm::vec2 pointNdc = glm::vec2(float(position.x()) / mOsgWidget->width(), float(position.y()) / mOsgWidget->height());
   sim::Vector3 camPosition = *getPosition(*mCurrentSimCamera);
   return pickPointOnPlanet(*mEngineRoot->simWorld, camPosition, glm::inverse(calcCurrentViewProjTransform()), pointNdc);
}

MainWindow::ViewportClickHandler MainWindow::getDefaultViewportClickHandler()
{
	static ViewportClickHandler f = [this] (Qt::MouseButton button, const QPointF& position) {
		if (button == Qt::MouseButton::LeftButton)
		{
			if (mViewportInput)
			{
				mViewportInput->setEnabled(true);
			}
		}
		else if (button == Qt::MouseButton::MiddleButton)
		{
			skybolt::sim::EntityId selectedEntityId = nullEntityId();
			auto hasNamePredicate = [] (const skybolt::sim::Entity& e) { return !getName(e).empty(); };
			
			if (std::optional<PickedSceneObject> object = pickSceneObjectAtPointInWindow(position, hasNamePredicate); object)
			{
				selectedEntityId = object->entity->getId();
			}
			if (selectedEntityId != mSelectedEntityId)
			{
				setSelectedEntity(selectedEntityId);
				sim::Entity* selectedEntity = mEngineRoot->simWorld->getEntityById(selectedEntityId);
				setPropertiesModel(selectedEntity ? std::make_shared<EntityPropertiesModel>(selectedEntity) : nullptr);
			}
		}
	};
	return f;
}

void MainWindow::onViewportMouseDown(Qt::MouseButton button, const QPointF& position)
{
	mViewportClickHandler(button, position);
}

void MainWindow::showContextMenu(const QPoint& point)
{
	QMenu contextMenu(tr("Context menu"), this);
	if (auto intersection = pickPointOnPlanetAtPointInWindow(point); intersection)
	{
		ActionContext context;
		context.entity = mEngineRoot->simWorld->getEntityById(mSelectedEntityId);
		context.point = *intersection;

		for (const auto& contextAction : mContextActions)
		{
			if (contextAction->handles(context))
			{
				auto action = new QAction(QString::fromStdString(contextAction->getName()), this);
				connect(action, &QAction::triggered, this, [&] { contextAction->execute(context); });
				contextMenu.addAction(action);
			}
		}
		if (!contextMenu.actions().empty())
		{
			contextMenu.exec(mOsgWidget->mapToGlobal(point));
		}
	}
}

void MainWindow::addToolWindow(const QString& windowName, QWidget* window)
{
	window->setWindowTitle(windowName);
	window->setObjectName(windowName);

	QAction* action = new QAction(windowName, ui->menuView);
	ui->menuView->insertAction(mViewMenuToolWindowSeparator, action);
	
	action->setData((int)mToolActions.size());
	action->setCheckable(true);
	action->setChecked(true);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(toolWindowActionToggled(bool)));
	mToolActions.push_back(action);

	mToolWindowManager->addToolWindow(window, ToolWindowManager::LastUsedArea);
}

void MainWindow::raiseToolWindow(QWidget* widget)
{
	ToolWindowManagerArea* area = mToolWindowManager->areaOf(widget);
	if (area)
	{
		area->activateWindow();
	}
}

void MainWindow::toolWindowActionToggled(bool state)
{
	int index = static_cast<QAction*>(sender())->data().toInt();
	QWidget *toolWindow = mToolWindowManager->toolWindows()[index];
	// TODO: restore tool to previous location
	mToolWindowManager->moveToolWindow(toolWindow, state ? ToolWindowManager::LastUsedArea : ToolWindowManager::NoArea);
}

void MainWindow::toolWindowVisibilityChanged(QWidget* toolWindow, bool visible)
{
	int index = mToolWindowManager->toolWindows().indexOf(toolWindow);
	mToolActions[index]->blockSignals(true);
	mToolActions[index]->setChecked(visible);
	mToolActions[index]->blockSignals(false);
}

void MainWindow::setProjectFilename(const QString& filename)
{
	if (!mProjectFilename.isEmpty())
	{
		;
		mSprocketModel->removeFileSearchPath(std::filesystem::path(mProjectFilename.toStdString()).parent_path().string());
	}

	mProjectFilename = filename;
	QString title = QApplication::applicationName();
	if (!mProjectFilename.isEmpty())
	{
		title += " - " + mProjectFilename;
	}
	setWindowTitle(title);

	mSprocketModel->addFileSearchPath(std::filesystem::path(mProjectFilename.toStdString()).parent_path().string());
}

void MainWindow::onEvent(const Event& event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		if (mouseEvent->type == MouseEvent::Type::Released)
		{
			mDisableInputSystemOnNextUpdate = true; // Can't disable it now because we are in the middle of the input capture process
		}
	}
}

static QAction* addCheckedAction(QMenu& menu, const QString& text, std::function<void(bool checked)> fn)
{
	QAction* action = menu.addAction(text, fn);
	action->setCheckable(true);
	return action;
}

QMenu* MainWindow::addVisibilityFilterableSubMenu(const QString& text, EntityVisibilityFilterable* filterable) const
{
	QMenu* menu = ui->menuView->addMenu(text);

	QActionGroup* alignmentGroup = new QActionGroup(menu);

	QAction* offAction = addCheckedAction(*menu, "Hide All", [=](bool checked) {
		filterable->setVisibilityPredicate(EntityVisibilityFilterable::visibilityOff);
	});

	auto hasLineOfSight = [this](const Entity& entity) {
		if (const auto& entityPosition = getPosition(entity); entityPosition)
		{
			// FIXME: We shouldn't assume position is in geocetric coordinates, or earth radius
			const auto cameraPosition = *getPosition(*mCurrentSimCamera);
			Vector3 camToEntityDir = *entityPosition - cameraPosition;
			double length = glm::length(camToEntityDir);
			camToEntityDir /= length;

			double effectivePlanetRadius = sim::earthRadius() - 10000; // use a slightly smaller radius for robustness
			return !intersectRaySegmentSphere(cameraPosition, camToEntityDir, length, Vector3(0,0,0), effectivePlanetRadius);
		}
		return false;
	};

	QAction* onAction = addCheckedAction(*menu, "Show All", [=](bool checked) {
		filterable->setVisibilityPredicate(hasLineOfSight);
	});

	QAction* selectedAction = addCheckedAction(*menu, "Show Selected", [=](bool checked) {
		filterable->setVisibilityPredicate([=](const Entity& entity) {
			return (mSelectedEntityId == entity.getId()) &&
				hasLineOfSight(entity);
		});
	});

	alignmentGroup->addAction(offAction);
	alignmentGroup->addAction(onAction);
	alignmentGroup->addAction(selectedAction);

	filterable->setVisibilityPredicate(hasLineOfSight);
	onAction->setChecked(true);

	return menu;
}

void MainWindow::addViewportMenuActions()
{
	addVisibilityFilterableSubMenu("Labels", mVisNameLabels.get());
	addVisibilityFilterableSubMenu("Forces", mForcesVisBinding.get());
	addVisibilityFilterableSubMenu("Orbits", mVisOrbits.get());
}

glm::dmat4 MainWindow::calcCurrentViewProjTransform() const
{
	skybolt::sim::Vector3 origin = *getPosition(*mCurrentSimCamera);
	skybolt::sim::Quaternion orientation = *getOrientation(*mCurrentSimCamera);
	skybolt::sim::CameraState camera = mCurrentSimCamera->getFirstComponentRequired<sim::CameraComponent>()->getState();
	double aspectRatio = double(mOsgWidget->width()) / double(mOsgWidget->height());
	return makeViewProjTransform(origin, orientation, camera, aspectRatio);
}
