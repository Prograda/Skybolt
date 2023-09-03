/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltCommon/Event.h>
#include <SkyboltSim/CameraController/CameraController.h>
#include <SkyboltSim/System/System.h>

#include <nlohmann/json.hpp>
#include <boost/signals2.hpp>

namespace skybolt {

enum class CameraInputAxisType
{
	Forward,
	Right
};

using CameraInputAxes = std::map<CameraInputAxisType, LogicalAxisPtr>;

//! This system listens to input events from the InputPlatform and emits a CameraController::Input signal.
//! This system should be updated after InputPlatform is updated.
class CameraInputSystem : public sim::System, public skybolt::EventListener
{
public:
	CameraInputSystem(const skybolt::InputPlatformPtr& inputPlatform, CameraInputAxes axes = {});
	~CameraInputSystem() override;

	void setMouseEnabled(bool enabled);
	void setKeyboardEnabled(bool enabled);

	//! @param rotationRadiansPerPixel gives camera rotation angle in radians for every pixel of mouse movement
	void setCameraRotationAnglePerMousePixel(float cameraRotationAnglePerMousePixel) { mCameraRotationAnglePerMousePixel = cameraRotationAnglePerMousePixel; }

	boost::signals2::signal<void(const skybolt::sim::CameraController::Input&)> cameraInputGenerated;

public: // SimUpdatable interface

	void advanceWallTime(sim::SecondsD newTime, sim::SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::BeginStateUpdate, generateCameraInput)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void generateCameraInput();

protected: // EventListener interface
	void onEvent(const skybolt::Event &evt) override;

protected:
	skybolt::InputPlatformPtr mInputPlatform;
	sim::SecondsD mDtWallClock = 0;

private:
	CameraInputAxes mAxes;

	skybolt::sim::CameraController::Input mInput = skybolt::sim::CameraController::Input::zero();
	float mCameraRotationAnglePerMousePixel = 1.f / 1000.f;
};

CameraInputAxes createDefaultCameraInputAxes(const skybolt::InputPlatform& inputPlatform);

void connectToCamera(CameraInputSystem& system, const sim::EntityPtr& camera);

void configure(CameraInputSystem& system, int screenHeightPixels, const nlohmann::json& engineSettings);

} // skybolt