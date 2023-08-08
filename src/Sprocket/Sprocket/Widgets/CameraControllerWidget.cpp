#include "CameraControllerWidget.h"
#include "Sprocket/Entity/EntityListModel.h"

#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QSortFilterProxyModel>

using namespace skybolt;

static bool isPlanet(const sim::Entity& entity)
{
	return sim::getPosition(entity).has_value() && entity.getFirstComponent<sim::PlanetComponent>() != nullptr;
}

static bool entityPredicateAlwaysFalse(const sim::Entity& entity)
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

CameraControllerWidget::CameraControllerWidget(sim::World* world, QWidget* parent) :
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

void CameraControllerWidget::setCamera(const sim::EntityPtr& camera)
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
		mCameraTargetCombo->setCurrentText(QString::fromStdString(sim::getName(*target)));
	}
		
	connect(mCameraTargetCombo, &QComboBox::currentTextChanged, [=](const QString& text)
	{
		sim::EntityPtr object = sim::findObjectByName(*mWorld, text.toStdString());
		if (object)
		{
			cameraControllerSelector->setTarget(object.get());
		}
	});

	mControllerConnections.push_back(cameraControllerSelector->targetChanged.connect([this](sim::Entity* target) {
		mCameraTargetCombo->blockSignals(true);
		mCameraTargetCombo->setCurrentText(target ? QString::fromStdString(sim::getName(*target)) : "");
		mCameraTargetCombo->blockSignals(false);
	}));
}

void CameraControllerWidget::updateTargetFilterForControllerName(const std::string& name)
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
