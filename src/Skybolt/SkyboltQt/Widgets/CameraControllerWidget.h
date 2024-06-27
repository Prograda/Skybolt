/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/World.h>

#include <QWidget>
#include <string>
#include <boost/signals2.hpp>

class EntityListModel;
class QComboBox;

class CameraControllerWidget : public QWidget, public skybolt::sim::WorldListener
{
public:
	CameraControllerWidget(skybolt::sim::World* world, QWidget* parent = nullptr);
	~CameraControllerWidget();

	void setCamera(skybolt::sim::Entity* camera);

	void update();

public:
	void entityRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	void updateTargetFilterForControllerName(const std::string& name);

private:
	skybolt::sim::World* mWorld;
	skybolt::sim::Entity* mCamera = nullptr;
	QComboBox* mCameraModeCombo;
	QComboBox* mCameraTargetCombo;
	EntityListModel* mTargetListModel;
	std::vector<boost::signals2::scoped_connection> mControllerConnections;
};