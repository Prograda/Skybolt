/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/World.h>

#include <QWidget>
#include <functional>
#include <string>

class QComboBox;

using EntityPredicate = std::function<bool(const skybolt::sim::Entity&)>;

class CameraControllerWidget : public QWidget
{
	Q_OBJECT
public:
	CameraControllerWidget(skybolt::sim::World* world, QWidget* parent = nullptr);
	~CameraControllerWidget();

	Q_SIGNAL void cameraSelectionChanged(const skybolt::sim::Entity* camera);

private:
	void updateModeComboSelectionFromModel();
	void updateTargetComboSelectionFromModel();
	void updateCameraSelectionFromUi();
	void updateTargetSelectionFromUi();

	void updateTargetFilterForControllerName(const skybolt::sim::Entity& camera, const std::string& name);

	skybolt::sim::CameraControllerComponent* getSelectedCameraControllerComponent() const;

private:
	skybolt::sim::World* mWorld;
	EntityPredicate mEntityPredicate;

	QComboBox* mCameraCombo;
	QComboBox* mCameraModeCombo;
	QComboBox* mCameraTargetCombo;

	skybolt::sim::EntityId mPrevCameraId = skybolt::sim::nullEntityId();
	skybolt::sim::EntityId mPrevTargetId = skybolt::sim::nullEntityId();
};