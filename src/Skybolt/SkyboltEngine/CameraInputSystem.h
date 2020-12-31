/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltCommon/Event.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/CameraController/CameraController.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

//! Applies user input to the camera each update
class CameraInputSystem : public sim::System, public EventListener
{
public:
	CameraInputSystem(const sim::EntityPtr& camera, const InputPlatformPtr& inputPlatform, const std::vector<LogicalAxisPtr>& axes);

	~CameraInputSystem();

	void updatePostDynamics(const sim::System::StepArgs& args) override;

	void setInputEnabled(bool enabled);

	static std::vector<LogicalAxisPtr> createDefaultAxes(const InputPlatform& inputPlatform);

private: // EventListener interface
	void onEvent(const Event &event) override;

private:
	const vis::Window* mWindow;
	sim::EntityPtr mCamera;
	InputPlatformPtr mInputPlatform;
	std::vector<LogicalAxisPtr> mInputAxes;
	sim::CameraController::Input mInput;
	bool mEnabled = true;
};

} // skybolt