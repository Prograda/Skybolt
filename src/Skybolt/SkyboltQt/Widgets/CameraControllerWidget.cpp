/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraControllerWidget.h"
#include "SkyboltQt/Entity/EntityListModel.h"

#include <SkyboltEngine/Components/TemplateNameComponent.h>
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

CameraControllerWidget::CameraControllerWidget(sim::World* world, QWidget* parent) :
	mWorld(world),
	QWidget(parent)
{
	mWorld->addListener(this);

	setLayout(new QHBoxLayout());
	mCameraModeCombo = new QComboBox();
	mCameraModeCombo->setToolTip("Camera Mode");
	mCameraModeCombo->setEnabled(false);
	layout()->addWidget(mCameraModeCombo);

	mTargetListModel = new EntityListModel(world, &entityPredicateAlwaysFalse, /* addBlankItem */ true);
	auto proxyModel = new QSortFilterProxyModel(this);
	proxyModel->sort(0);
	proxyModel->setSourceModel(mTargetListModel);

	mCameraTargetCombo = new QComboBox();
	mCameraTargetCombo->setToolTip("Camera Target");
	mCameraTargetCombo->setModel(proxyModel);
	mCameraTargetCombo->setEnabled(false);
	layout()->addWidget(mCameraTargetCombo);
}

CameraControllerWidget::~CameraControllerWidget()
{
	mWorld->removeListener(this);
}

void CameraControllerWidget::setCamera(sim::Entity* camera)
{
	mCamera = camera;
	sim::CameraControllerComponentPtr cameraController = camera ? camera->getFirstComponent<sim::CameraControllerComponent>() : nullptr;

	// Clear
	mCameraModeCombo->disconnect();
	mCameraTargetCombo->disconnect();
	mCameraModeCombo->clear();
	mControllerConnections.clear();

	if (!cameraController)
	{
		mCameraModeCombo->setEnabled(false);
		mCameraTargetCombo->setEnabled(false);
		mTargetListModel->setEntityFilter(&entityPredicateAlwaysFalse);
		return;
	}

	mCameraModeCombo->setEnabled(true);
	mCameraTargetCombo->setEnabled(true);

	// Mode combo
	for (const auto& item : cameraController->getControllers())
	{
		mCameraModeCombo->addItem(QString::fromStdString(item.first));
	}

	const std::string& currentControllerName = cameraController->getSelectedControllerName();
	mCameraModeCombo->setCurrentText(QString::fromStdString(currentControllerName));
	updateTargetFilterForControllerName(currentControllerName);

	connect(mCameraModeCombo, &QComboBox::currentTextChanged, [=](const QString& text)
	{
		cameraController->selectController(text.toStdString());
	});

	mControllerConnections.push_back(cameraController->controllerSelected.connect([this](const std::string& name) {
		mCameraModeCombo->blockSignals(true);
		mCameraModeCombo->setCurrentText(QString::fromStdString(name));
		mCameraModeCombo->blockSignals(false);
		updateTargetFilterForControllerName(name);
	}));

	// Target combo
	const sim::EntityId& targetId = cameraController->getTargetId();
	sim::Entity* target = mWorld->getEntityById(targetId).get();
	mCameraTargetCombo->setCurrentText(QString::fromStdString(target ? sim::getName(*target) : ""));
		
	connect(mCameraTargetCombo, &QComboBox::currentTextChanged, [=](const QString& text)
	{
		sim::Entity* entity = mWorld->findObjectByName(text.toStdString()).get();
		if (entity)
		{
			cameraController->setTargetId(entity->getId());
		}
	});
}

void CameraControllerWidget::update()
{
	if (!mCamera)
	{
		return;
	}

	QString newTargetName;
	if (sim::CameraControllerComponentPtr cameraController = mCamera->getFirstComponent<sim::CameraControllerComponent>(); cameraController)
	{
		if (sim::Entity* target = mWorld->getEntityById(cameraController->getTargetId()).get(); target)
		{
			newTargetName = QString::fromStdString(sim::getName(*target));
		}
	}

	if (newTargetName != mCameraTargetCombo->currentText())
	{
		mCameraTargetCombo->blockSignals(true);
		mCameraTargetCombo->setCurrentText(newTargetName);
		mCameraTargetCombo->blockSignals(false);
	}
}

void CameraControllerWidget::entityRemoved(const sim::EntityPtr& entity)
{
	if (mCamera == entity.get())
	{
		setCamera(nullptr);
	}
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
