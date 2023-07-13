/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportWidget.h"
#include "CaptureImageDailog.h"
#include "Sprocket/SceneSelectionModel.h"
#include "Sprocket/Entity/EntityListModel.h"
#include "Sprocket/Icon/SprocketIcons.h"
#include "Sprocket/Property/PropertyEditor.h"
#include "Sprocket/Scenario/EntityObjectType.h"
#include "Sprocket/QtUtil/QtDialogUtil.h"
#include "Sprocket/Viewport/ViewportInput.h"
#include "Sprocket/Viewport/ViewportPropertiesModel.h"
#include "Sprocket/Viewport/OsgWidget.h"
#include "Sprocket/Widgets/CameraControllerWidget.h"

#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVis/Window/CaptureScreenshot.h>
#include <SkyboltVis/Window/Window.h>

#include <osg/Texture2D>
#include <QBoxLayout>
#include <QComboBox>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

using namespace skybolt;

ViewportWidget::ViewportWidget(const ViewportWidgetConfig& config) :
	mEngineRoot(config.engineRoot),
	mViewportInput(config.viewportInput),
	mSelectionModel(config.selectionModel),
	mContextActions(config.contextActions),
	mFilterMenu(new QMenu(this)),
	QWidget(config.parent)
{
	assert(mEngineRoot);

	mOsgWidget = new OsgWidget(config.visRoot, this);
	mOsgWidget->setMouseTracking(true);

	mToolBar = createViewportToolBar(config.projectFilenameGetter);

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
	connect(mOsgWidget, &OsgWidget::customContextMenuRequested, this, [this] (const QPoint& point) { showContextMenu(point); });

	connect(mOsgWidget, &OsgWidget::mousePressed, this, [this](const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mousePressed(position, button, modifiers))
			{
				break;
			}
		}
	});

	connect(mOsgWidget, &OsgWidget::mouseReleased, this, [this](const QPointF& position, Qt::MouseButton button) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mouseReleased(position, button))
			{
				break;
			}
		}
	});

	connect(mOsgWidget, &OsgWidget::mouseMoved, this, [this](const QPointF& position, Qt::MouseButtons buttons) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mouseMoved(position, buttons))
			{
				break;
			}
		}
	});

	// Create center layout
	{
		QBoxLayout* layout = new QVBoxLayout();
		layout->setMargin(0);
		setLayout(layout);

		layout->addWidget(mToolBar);
		layout->addWidget(mOsgWidget);
	}

	mSceneObjectPicker = createSceneObjectPicker(&mEngineRoot->scenario->world);
}

ViewportWidget::~ViewportWidget() = default;

void ViewportWidget::update()
{
	mOsgWidget->update(); // TODO only redraw if something changed

	auto simVisSystem = sim::findSystem<SimVisSystem>(*mEngineRoot->systemRegistry);
	assert(simVisSystem);
	const GeocentricToNedConverter& coordinateConverter = simVisSystem->getCoordinateConverter();

	if (mCurrentSimCamera)
	{
		simVisSystem->setSceneOriginProvider(SimVisSystem::sceneOriginFromEntity(&mEngineRoot->scenario->world, mCurrentSimCamera->getId()));

		if (auto controller = mCurrentSimCamera->getFirstComponent<sim::CameraControllerComponent>(); controller)
		{
			if (controller->getSelectedController())
			{
				controller->getSelectedController()->setInput(mViewportInput->getInput());
			}
		}
	}
}

void ViewportWidget::captureImage(const std::filesystem::path& baseFilename)
{
	mOsgWidget->setFixedWidth(1920);
	mOsgWidget->setFixedHeight(1080);

	QString defaultSequenceName = "untitled";
	if (!baseFilename.empty())
	{
		defaultSequenceName = QString::fromStdString(baseFilename.stem().string());
	}

	showCaptureImageSequenceDialog([=](double time, const QString& filename) {
			mEngineRoot->scenario->timeSource.setTime(time);
			update();
			bool fullyLoadEachFrameBeforeProgressing = false;
			if (fullyLoadEachFrameBeforeProgressing)
			{
				while (mEngineRoot->stats.terrainTileLoadQueueSize > 0)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(1ms);
					mOsgWidget->update();
				}
			}
			QImage image = mOsgWidget->grabFramebuffer();
			image.save(filename);
		}, defaultSequenceName, this);

	mOsgWidget->setMinimumSize(1, 1);
	mOsgWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

std::optional<PickedSceneObject> ViewportWidget::pickSceneObjectAtPointInWindow(const QPointF& position, const EntitySelectionPredicate& predicate) const
{
	glm::vec2 pointNdc = glm::vec2(float(position.x()) / mOsgWidget->width(), float(position.y()) / mOsgWidget->height());
	glm::dmat4 transform = calcCurrentViewProjTransform();
	return mSceneObjectPicker(transform, pointNdc, 0.04, predicate);
}

std::optional<sim::Vector3> ViewportWidget::pickPointOnPlanetAtPointInWindow(const QPointF& position) const
{
	if (mCurrentSimCamera)
	{
		glm::vec2 pointNdc = glm::vec2(position.x() / mOsgWidget->width(), position.y() / mOsgWidget->height());
		sim::Vector3 camPosition = *getPosition(*mCurrentSimCamera);
		std::optional<PickedSceneObject> object = pickPointOnPlanet(mEngineRoot->scenario->world, camPosition, glm::inverse(calcCurrentViewProjTransform()), pointNdc);
		return object ? object->position : std::optional<sim::Vector3>();
	}
	return std::nullopt;
}

void ViewportWidget::addMouseEventHandler(const ViewportMouseEventHandlerPtr& handler, int priority)
{
	mMouseEventHandlers.insert(std::make_pair(priority, std::move(handler)));
}

void ViewportWidget::removeMouseEventHandler(const ViewportMouseEventHandler& handler)
{
	for (auto i = mMouseEventHandlers.begin(); i != mMouseEventHandlers.end(); ++i)
	{
		if (i->second.get() == &handler)
		{
			mMouseEventHandlers.erase(i);
			return;
		}
	}
}

void ViewportWidget::showContextMenu(const QPoint& point)
{
	QMenu contextMenu(tr("Context menu"), this);
	if (auto intersection = pickPointOnPlanetAtPointInWindow(point); intersection)
	{
		sim::Entity* selectedEntity = nullptr;
		if (auto item = getFirstSelectedScenarioObjectOfType<EntityObject>(mSelectionModel->getSelectedItems()); item)
		{
			selectedEntity = mEngineRoot->scenario->world.getEntityById(item->data);
		}

		ActionContext context;
		context.entity = selectedEntity;
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

glm::dmat4 ViewportWidget::calcCurrentViewProjTransform() const
{
	if (mCurrentSimCamera)
	{
		sim::Vector3 origin = *getPosition(*mCurrentSimCamera);
		sim::Quaternion orientation = *getOrientation(*mCurrentSimCamera);
		sim::CameraState camera = mCurrentSimCamera->getFirstComponentRequired<sim::CameraComponent>()->getState();
		double aspectRatio = double(mOsgWidget->width()) / double(mOsgWidget->height());
		return makeViewProjTransform(origin, orientation, camera, aspectRatio);
	}
	return math::dmat4Identity();
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

void ViewportWidget::setCamera(sim::Entity* simCamera)
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

void ViewportWidget::setCameraTarget(sim::Entity* target)
{
	if (mCurrentSimCamera)
	{
		vis::CameraPtr visCamera = getFirstVisCamera(*mCurrentSimCamera);
		if (auto controller = mCurrentSimCamera->getFirstComponent<sim::CameraControllerComponent>(); controller)
		{
			if (controller->getSelectedController())
			{
				controller->getSelectedController()->setTarget(target);
			}
		}
	}
}

QToolBar* ViewportWidget::createViewportToolBar(const std::function<std::string()>& projectFilenameGetter)
{
	QToolBar* toolbar = new QToolBar(this);

	sim::World* world = &mEngineRoot->scenario->world;

	{
		mCameraCombo = new QComboBox();
		mCameraCombo->setToolTip("Camera");
		mCameraCombo->setModel(new EntityListModel(world, [] (const sim::Entity& entity) {
			return entity.getFirstComponent<sim::CameraComponent>() != nullptr;
		}));

		connect(mCameraCombo, &QComboBox::currentTextChanged, [=](const QString& text)
		{
			sim::Entity* object = world->findObjectByName(text.toStdString());
			setCamera(object);
		});

		toolbar->addWidget(mCameraCombo);

		mCameraControllerWidget = new CameraControllerWidget(world);
		toolbar->addWidget(mCameraControllerWidget);
	}

	toolbar->addAction(getSprocketIcon(SprocketIcon::Screenshot), "Capture Image", [this, projectFilenameGetter] { captureImage(projectFilenameGetter()); });

	{
		QToolButton* toolButton = new QToolButton(this);
		toolButton->setText("Filter");
		toolButton->setToolTip("Filter");
		toolButton->setIcon(getSprocketIcon(SprocketIcon::Filter));
		toolButton->setMenu(mFilterMenu);
		toolButton->setPopupMode(QToolButton::InstantPopup);
		toolbar->addWidget(toolButton);
	}

	toolbar->addAction(getSprocketIcon(SprocketIcon::Settings), "Settings", [=] {
		if (mCurrentSimCamera)
		{
			PropertyEditor* editor = new PropertyEditor({});
			editor->setModel(std::make_shared<ViewportPropertiesModel>(mEngineRoot->scene.get(), mCurrentSimCamera->getFirstComponent<sim::CameraComponent>().get()));
			QDialog* dialog = createDialogNonModal(editor, "Settings", this);
			dialog->exec();
		}
	});

	return toolbar;
}

void ViewportWidget::resetProject()
{
	mEngineRoot->scene->setAmbientLightColor(osg::Vec3f(0,0,0));
}

static osg::Vec3f toOsgVec3f(const sim::Vector3& v)
{
	return osg::Vec3f(v.x, v.y, v.z);
}

static sim::Vector3 toSimVector3(const osg::Vec3f& v)
{
	return sim::Vector3(v.x(), v.y(), v.z());
}

void ViewportWidget::readProject(const nlohmann::json& projectJson)
{
	if (auto i = projectJson.find("viewport"); i != projectJson.end())
	{
		const nlohmann::json& child = i.value();
		{
			auto it = child.find("camera");
			if (it != child.end())
			{
				std::string name = it.value();
				sim::Entity* camera = mEngineRoot->scenario->world.findObjectByName(name);
				setCamera(camera);
			}
		}
		{
			auto it = child.find("ambientLight");
			if (it != child.end())
			{
				mEngineRoot->scene->setAmbientLightColor(toOsgVec3f(sim::readVector3(it.value())));
			}
		}
	}
	else
	{
		resetProject();
	}
}

void ViewportWidget::writeProject(nlohmann::json& json) const
{
	if (mCurrentSimCamera)
	{
		nlohmann::json& child = json["viewport"];
		child["camera"] = getName(*mCurrentSimCamera);
		child["ambientLight"] = sim::writeJson(toSimVector3(mEngineRoot->scene->getAmbientLightColor()));
	}
}

void ViewportWidget::entityRemoved(const sim::EntityPtr& entity)
{
	if (mCurrentSimCamera == entity.get())
	{
		setCamera(nullptr);
	}
}

static QAction* addCheckedAction(QMenu& menu, const QString& text, std::function<void(bool checked)> fn)
{
	QAction* action = menu.addAction(text, fn);
	action->setCheckable(true);
	return action;
}

QMenu* ViewportWidget::addVisibilityFilterableSubMenu(const QString& text, const EntityVisibilityPredicateSetter& setter) const
{
	QMenu* menu = mFilterMenu->addMenu(text);
	QActionGroup* alignmentGroup = new QActionGroup(menu);

	QAction* offAction = addCheckedAction(*menu, "Hide All", [=](bool checked) {
		setter(EntityVisibilityFilterable::visibilityOff);
	});

	auto hasLineOfSight = [this](const sim::Entity& entity) {
		if (const auto& entityPosition = getPosition(entity); entityPosition)
		{
			if (mCurrentSimCamera)
			{
				sim::Vector3 cameraPosition = *getPosition(*mCurrentSimCamera);
				if (sim::Entity* planet = sim::findNearestEntityWithComponent<sim::PlanetComponent>(mEngineRoot->scenario->world.getEntities(), cameraPosition).get(); planet)
				{
					const sim::PlanetComponent& planetComponent = *planet->getFirstComponent<sim::PlanetComponent>();

					sim::Vector3 planetPosition = *getPosition(*planet);
					sim::Vector3 camToEntityDir = *entityPosition - cameraPosition;
					double length = glm::length(camToEntityDir);
					camToEntityDir /= length;

					double effectivePlanetRadius = std::max(0.0, planetComponent.radius * 0.999); // use a slightly smaller radius for robustness
					return !intersectRaySegmentSphere(cameraPosition, camToEntityDir, length, planetPosition, effectivePlanetRadius);
				}
			}
		}
		return true;
	};

	QAction* onAction = addCheckedAction(*menu, "Show All", [=](bool checked) {
		setter(hasLineOfSight);
	});

	QAction* selectedAction = addCheckedAction(*menu, "Show Selected", [=](bool checked) {
		setter([=](const sim::Entity& entity) {
			for (const auto& object : mSelectionModel->getSelectedItems())
			{
				if (auto entityObject = dynamic_cast<EntityObject*>(object.get()); entityObject)
				{
					return (entityObject->data == entity.getId()) && hasLineOfSight(entity);
				}
			}
			return false;
		});
	});

	alignmentGroup->addAction(offAction);
	alignmentGroup->addAction(onAction);
	alignmentGroup->addAction(selectedAction);

	setter(hasLineOfSight);
	onAction->setChecked(true);

	return menu;
}

int ViewportWidget::getViewportWidth() const
{
	return mOsgWidget->width();
}

int ViewportWidget::getViewportHeight() const
{
	return mOsgWidget->height();
}