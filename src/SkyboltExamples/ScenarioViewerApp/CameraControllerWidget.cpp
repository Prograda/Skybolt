/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraControllerWidget.h"

#include <SkyboltEngine/Components/TemplateNameComponent.h>
#include <SkyboltWidgets/List/SyncedListModel.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/CameraComponent.h>
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

static bool entityPredicateIsCamera(const sim::Entity& entity)
{
	return entity.getFirstComponent<sim::CameraComponent>() != nullptr;
}

static QSortFilterProxyModel* wrapModelWithSortFilter(QAbstractItemModel* sourceModel, QObject* parent)
{
	auto proxyModel = new QSortFilterProxyModel(parent);
	proxyModel->setSourceModel(sourceModel);
	proxyModel->sort(0);
	return proxyModel;
}

static std::set<QString> getFilteredEntities(const sim::World& world, const EntityPredicate& entityPredicate)
{
	std::set<QString> items;
	for (const auto& entity : world.getEntities())
	{
		if (entityPredicate(*entity))
		{
			items.insert(QString::fromStdString(sim::getName(*entity)));
		}
	}
	return items;
}

CameraControllerWidget::CameraControllerWidget(sim::World* world, QWidget* parent) :
	mWorld(world),
	mEntityPredicate(entityPredicateAlwaysFalse),
	QWidget(parent)
{
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);

	auto camerasGetter = [this] {
		return getFilteredEntities(*mWorld, entityPredicateIsCamera);
		};

	auto entitiesGetter = [this] {
		return getFilteredEntities(*mWorld, mEntityPredicate);
		};

	auto cameraModesGetter = [this]() {
		std::set<QString> items;
		
		if (auto cameraController = getSelectedCameraControllerComponent(); cameraController)
		{
			for (const auto& i : cameraController->getControllers())
			{
				items.insert(QString::fromStdString(i.first));
			}
		}
		return items;
		};

	// Add camera selection combo
	mCameraCombo = new QComboBox();
	mCameraCombo->setToolTip("Camera Mode");
	mCameraCombo->setModel(wrapModelWithSortFilter(new SyncedListModel(camerasGetter), this));
	layout()->addWidget(mCameraCombo);

	connect(mCameraCombo, &QComboBox::textActivated, [this](const QString& text)
	{
		updateModeComboSelectionFromModel();
		updateCameraSelectionFromUi();
	});

	// Add camera mode selection combo
	mCameraModeCombo = new QComboBox();
	mCameraModeCombo->setToolTip("Camera Mode");
	mCameraModeCombo->setModel(wrapModelWithSortFilter(new SyncedListModel(cameraModesGetter), this));
	layout()->addWidget(mCameraModeCombo);

	connect(mCameraModeCombo, &QComboBox::textActivated, [=](const QString& text)
	{
		if (auto component = getSelectedCameraControllerComponent(); component)
		{
			component->selectController(text.toStdString());
		}
		updateTargetComboSelectionFromModel();
	});

	// Add camera target selection combo
	mCameraTargetCombo = new QComboBox();
	mCameraTargetCombo->setToolTip("Camera Target");
	mCameraTargetCombo->setModel(wrapModelWithSortFilter(new SyncedListModel(entitiesGetter), this));
	mCameraTargetCombo->setMinimumContentsLength(10);
	layout()->addWidget(mCameraTargetCombo);

	// Set camera controller target if the user changed the QComboBox selection
	connect(mCameraTargetCombo, &QComboBox::textActivated, this, [=](const QString& text) {
		updateTargetSelectionFromUi();
	});

	updateModeComboSelectionFromModel();

	// Update periodically
	createAndStartIntervalTimer(100, this, [this] {
		updateModeComboSelectionFromModel();
		updateCameraSelectionFromUi();
		updateTargetSelectionFromUi();
		});
}

CameraControllerWidget::~CameraControllerWidget() = default;

void CameraControllerWidget::updateModeComboSelectionFromModel()
{
	mEntityPredicate = &entityPredicateAlwaysFalse;

	sim::EntityPtr camera = mWorld->findObjectByName(mCameraCombo->currentText().toStdString());
	if (!camera)
	{
		return;
	}

	sim::CameraControllerComponentPtr cameraController = camera->getFirstComponent<sim::CameraControllerComponent>();
	if (!cameraController)
	{
		return;
	}

	const std::string& currentControllerName = cameraController->getSelectedControllerName();
	mCameraModeCombo->setCurrentText(QString::fromStdString(currentControllerName));
	updateTargetFilterForControllerName(*camera, currentControllerName);
	updateTargetComboSelectionFromModel();
}

void CameraControllerWidget::updateTargetComboSelectionFromModel()
{
	auto cameraController = getSelectedCameraControllerComponent();
	if (!cameraController)
	{
		return;
	}

	const sim::EntityId& targetId = cameraController->getTargetId();
	sim::Entity* target = mWorld->getEntityById(targetId).get();
	mCameraTargetCombo->setCurrentText(QString::fromStdString(target ? sim::getName(*target) : ""));
}

void CameraControllerWidget::updateTargetFilterForControllerName(const sim::Entity& camera, const std::string& name)
{
	if (name == "Globe")
	{
		mEntityPredicate = &isPlanet;
	}
	else if (name == "Follow" || name == "Cockpit")
	{
		mEntityPredicate = ([cameraName = getName(camera)] (const sim::Entity& entity) {
			return getPosition(entity).has_value()
				&& entity.getFirstComponent<TemplateNameComponent>() != nullptr
				&& !isPlanet(entity)
				&& !entityPredicateIsCamera(entity)
				&& getName(entity) != cameraName;
		});
	}
}

void CameraControllerWidget::updateCameraSelectionFromUi()
{
	sim::EntityPtr camera = mWorld->findObjectByName(mCameraCombo->currentText().toStdString());
	sim::EntityId cameraId = camera ? camera->getId() : sim::nullEntityId();
	if (cameraId != mPrevCameraId)
	{
		mPrevCameraId = cameraId;
		emit cameraSelectionChanged(camera.get());
	}

}

void CameraControllerWidget::updateTargetSelectionFromUi()
{
	sim::EntityPtr target = mWorld->findObjectByName(mCameraTargetCombo->currentText().toStdString());
	sim::EntityId targetId = target ? target->getId() : sim::nullEntityId();
	if (targetId != mPrevTargetId)
	{
		mPrevTargetId = targetId;
		if (auto cameraController = getSelectedCameraControllerComponent(); cameraController)
		{
			cameraController->setTargetId(targetId);
		}
	}
}

skybolt::sim::CameraControllerComponent* CameraControllerWidget::getSelectedCameraControllerComponent() const
{
	sim::EntityPtr camera = mWorld->findObjectByName(mCameraCombo->currentText().toStdString());
	if (!camera)
	{
		return nullptr;
	}

	return camera->getFirstComponent<sim::CameraControllerComponent>().get();
}
