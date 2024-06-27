/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportWidget.h"
#include "CaptureImageDailog.h"
#include "SkyboltQt/Entity/EntityListModel.h"
#include "SkyboltQt/Icon/SkyboltIcons.h"
#include "SkyboltQt/Property/PropertyEditor.h"
#include "SkyboltQt/Scenario/EntityObjectType.h"
#include "SkyboltQt/Scenario/ScenarioSelectionModel.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include "SkyboltQt/Viewport/ViewportPropertiesModel.h"
#include "SkyboltQt/Viewport/OsgWindow.h"
#include "SkyboltQt/Viewport/PlanetPointPicker.h"
#include "SkyboltQt/Viewport/ScreenTransformUtil.h"
#include "SkyboltQt/Widgets/CameraControllerWidget.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVis/Window/CaptureScreenshot.h>
#include <SkyboltVis/Window/Window.h>

#include <osg/Texture2D>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QToolButton>

using namespace skybolt;

static sim::Entity* getSelectedCamera(const sim::World& world, const QComboBox& comboBox)
{
	return world.findObjectByName(comboBox.currentText().toStdString()).get();
}

ViewportWidget::ViewportWidget(const ViewportWidgetConfig& config) :
	mEngineRoot(config.engineRoot),
	mVisRoot(config.visRoot),
	mSelectionModel(config.selectionModel),
	mScenarioObjectPicker(config.scenarioObjectPicker),
	mContextActions(config.contextActions),
	mFilterMenu(new QMenu(this)),
	QWidget(config.parent)
{
	assert(mEngineRoot);
	assert(mVisRoot);
	assert(mSelectionModel);
	assert(mScenarioObjectPicker);

	setObjectName(config.viewportName);

	mOsgWindow = new OsgWindow(config.visRoot);

	mToolBar = createViewportToolBar(config.scenarioFilenameGetter);

	mViewport = new vis::DefaultRenderCameraViewport([&]{
		vis::DefaultRenderCameraViewportConfig c;
		c.scene = mEngineRoot->scene;
		c.programs = &mEngineRoot->programs;
		c.shadowParams = getShadowParams(mEngineRoot->engineSettings);
		c.cloudRenderingParams = getCloudRenderingParams(mEngineRoot->engineSettings);
		return c;
	}());
	mOsgWindow->getWindow()->getRenderOperationSequence().addOperation(mViewport);
	mOsgWidget = QWidget::createWindowContainer(mOsgWindow, this);

	sim::World* world = &mEngineRoot->scenario->world;
	setCamera(getSelectedCamera(*world, *mCameraCombo));

	connect(mOsgWindow, &OsgWindow::mousePressed, this, [this](const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mousePressed(*this, position, button, modifiers))
			{
				return;
			}
		}
	});

	connect(mOsgWindow, &OsgWindow::mouseReleased, this, [this](const QPointF& position, Qt::MouseButton button) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mouseReleased(*this, position, button))
			{
				return;
			}
		}
		if (button == Qt::RightButton)
		{
			showContextMenu(QPoint(position.x(), position.y()));
		}
	});

	connect(mOsgWindow, &OsgWindow::mouseMoved, this, [this](const QPointF& position, Qt::MouseButtons buttons) {
		for (const auto& [priority, handler] : mMouseEventHandlers)
		{
			if (handler->mouseMoved(*this, position, buttons))
			{
				return;
			}
		}
	});

	// Create center layout
	{
		QBoxLayout* layout = new QVBoxLayout();
		layout->setMargin(0);
		layout->setSpacing(0);
		setLayout(layout);

		layout->addWidget(mToolBar);
		layout->addWidget(mOsgWidget);
	}
}

ViewportWidget::~ViewportWidget() = default;

void ViewportWidget::update()
{
	mCameraControllerWidget->update();

	// Apply the OsgWidget cursor to the OsgWindow, as the window doesn't inheret the wrapper widget's cursor automatically
	Qt::CursorShape cursorShape = QApplication::overrideCursor() ? QApplication::overrideCursor()->shape() : mOsgWidget->cursor().shape();
	mOsgWindow->setCursor(QCursor(cursorShape));

	// Update scene origin
	auto simVisSystem = sim::findSystem<SimVisSystem>(*mEngineRoot->systemRegistry);
	assert(simVisSystem);
	const GeocentricToNedConverter& coordinateConverter = simVisSystem->getCoordinateConverter();

	if (mCurrentSimCamera)
	{
		simVisSystem->setSceneOriginProvider(SimVisSystem::sceneOriginFromEntity(&mEngineRoot->scenario->world, mCurrentSimCamera->getId()));
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

			mVisRoot->render();

			bool fullyLoadEachFrameBeforeProgressing = false;
			if (fullyLoadEachFrameBeforeProgressing)
			{
				while (mEngineRoot->stats.terrainTileLoadQueueSize > 0)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(1ms);
					mVisRoot->render();
				}
			}
			vis::captureScreenshot(*mVisRoot, filename.toStdString());
		}, defaultSequenceName, this);

	mOsgWidget->setMinimumSize(1, 1);
	mOsgWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

std::vector<PickedScenarioObject> ViewportWidget::pickSceneObjectsAtPointInWindow(const QPointF& position, const ScenarioObjectPredicate& predicate) const
{
	if (mCurrentSimCamera)
	{
		sim::Vector3 camPosition = *getPosition(*mCurrentSimCamera);
		glm::vec2 pointNdc = glm::vec2(float(position.x()) / mOsgWidget->width(), float(position.y()) / mOsgWidget->height());
		glm::dmat4 transform = calcCurrentViewProjTransform();
		return mScenarioObjectPicker(camPosition, transform, pointNdc, predicate);
	}
	return {};
}

std::optional<sim::Vector3> ViewportWidget::pickPointOnPlanetAtPointInWindow(const QPointF& position) const
{
	if (mCurrentSimCamera)
	{
		sim::Vector3 camPosition = *getPosition(*mCurrentSimCamera);
		glm::vec2 pointNdc = glm::vec2(position.x() / mOsgWidget->width(), position.y() / mOsgWidget->height());
		std::optional<PickedEntity> object = pickPointOnPlanet(mEngineRoot->scenario->world, camPosition, glm::inverse(calcCurrentViewProjTransform()), pointNdc);
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
	auto menu = new QMenu(tr("Context menu"), this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (auto intersection = pickPointOnPlanetAtPointInWindow(point); intersection)
	{
		sim::Entity* selectedEntity = nullptr;
		if (auto item = getFirstSelectedScenarioObjectOfType<EntityObject>(mSelectionModel->getSelectedItems()); item)
		{
			selectedEntity = mEngineRoot->scenario->world.getEntityById(item->data).get();
		}

		ActionContext context;
		context.entity = selectedEntity;
		context.point = *intersection;
		context.widget = this;

		for (const auto& contextAction : mContextActions)
		{
			if (contextAction->handles(context))
			{
				auto action = new QAction(QString::fromStdString(contextAction->getName()), this);
				connect(action, &QAction::triggered, this, [contextAction, context] () mutable { contextAction->execute(context); });
				menu->addAction(action);
			}
		}
		if (!menu->actions().empty())
		{
			menu->popup(mOsgWidget->mapToGlobal(point));
		}
		else
		{
			menu->deleteLater();
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

QToolBar* ViewportWidget::createViewportToolBar(const std::function<std::string()>& scenarioFilenameGetter)
{
	QToolBar* toolbar = new QToolBar(this);

	sim::World* world = &mEngineRoot->scenario->world;

	{
		auto cameraListModel = new EntityListModel(world, [] (const sim::Entity& entity) {
			return entity.getFirstComponent<sim::CameraComponent>() != nullptr;
		});

		auto proxyModel = new QSortFilterProxyModel(this);
		proxyModel->sort(0);
		proxyModel->setSourceModel(cameraListModel);

		mCameraCombo = new QComboBox();
		mCameraCombo->setToolTip("Camera");
		mCameraCombo->setModel(proxyModel);
		toolbar->addWidget(mCameraCombo);

		mCameraControllerWidget = new CameraControllerWidget(world);
		toolbar->addWidget(mCameraControllerWidget);

		connect(mCameraCombo, &QComboBox::currentTextChanged, [=](const QString& text) {
			setCamera(getSelectedCamera(*world, *mCameraCombo));
		});
	}

	toolbar->addAction(getSkyboltIcon(SkyboltIcon::Screenshot), "Capture Image", [this, scenarioFilenameGetter] { captureImage(scenarioFilenameGetter()); });

	{
		QToolButton* toolButton = new QToolButton(this);
		toolButton->setText("Filter");
		toolButton->setToolTip("Filter");
		toolButton->setIcon(getSkyboltIcon(SkyboltIcon::Filter));
		toolButton->setMenu(mFilterMenu);
		toolButton->setPopupMode(QToolButton::InstantPopup);
		toolbar->addWidget(toolButton);
	}

	toolbar->addAction(getSkyboltIcon(SkyboltIcon::Settings), "Settings", [=] {
		if (mCurrentSimCamera)
		{
			PropertyEditor* editor = new PropertyEditor();
			editor->setModel(std::make_shared<ViewportPropertiesModel>(mEngineRoot->scene.get(), mCurrentSimCamera->getFirstComponent<sim::CameraComponent>().get()));
			QDialog* dialog = createDialogNonModal(editor, "Settings", this);
			dialog->exec();
		}
	});

	return toolbar;
}

void ViewportWidget::resetScenario()
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

void ViewportWidget::readScenario(const nlohmann::json& projectJson)
{
	if (auto i = projectJson.find("viewports"); i != projectJson.end())
	{
		const nlohmann::json& viewportsChild = i.value();
		if (auto i = viewportsChild.find(objectName().toStdString()); i != viewportsChild.end())
		{
			const nlohmann::json& viewportJson = i.value();
			{
				auto it = viewportJson.find("camera");
				if (it != viewportJson.end())
				{
					std::string name = it.value();
					sim::Entity* camera = mEngineRoot->scenario->world.findObjectByName(name).get();
					setCamera(camera);
				}
			}
			{
				auto it = viewportJson.find("ambientLight");
				if (it != viewportJson.end())
				{
					mEngineRoot->scene->setAmbientLightColor(toOsgVec3f(sim::readVector3(it.value())));
				}
			}
		}
	}
	else
	{
		resetScenario();
	}
}

void ViewportWidget::writeScenario(nlohmann::json& json) const
{
	if (mCurrentSimCamera)
	{
		nlohmann::json& viewportsJson = json["viewports"];
		nlohmann::json& viewportJson = viewportsJson[objectName().toStdString()];
		viewportJson["camera"] = getName(*mCurrentSimCamera);
		viewportJson["ambientLight"] = sim::writeJson(toSimVector3(mEngineRoot->scene->getAmbientLightColor()));
	}
}

void ViewportWidget::entityRemoved(const sim::EntityPtr& entity)
{
	if (mCurrentSimCamera == entity.get())
	{
		setCamera(nullptr);
	}
}

int ViewportWidget::getViewportWidth() const
{
	return mOsgWidget->width();
}

int ViewportWidget::getViewportHeight() const
{
	return mOsgWidget->height();
}