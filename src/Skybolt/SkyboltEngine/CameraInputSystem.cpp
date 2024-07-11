/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraInputSystem.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltEngine/Input/LogicalAxis.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>

#include <boost/log/trivial.hpp>

namespace skybolt {

CameraInputSystem::CameraInputSystem(const skybolt::InputPlatformPtr& inputPlatform, CameraInputAxes axes) :
	mInputPlatform(inputPlatform),
	mAxes(std::move(axes))
{
	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);
}

CameraInputSystem::~CameraInputSystem()
{
	mInputPlatform->getEventEmitter()->removeEventListener(this);
}

void CameraInputSystem::setMouseEnabled(bool enabled)
{
	for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeMouse))
	{
		device->setEnabled(enabled);
	}
}

void CameraInputSystem::setKeyboardEnabled(bool enabled)
{
	for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard))
	{
		device->setEnabled(enabled);
	}
}

static float getAxisValueOrDefault(const CameraInputAxes& axes, CameraInputAxisType type, float defaultValue)
{
	if (auto a = findOptional(axes, type); a)
	{
		return (*a)->getState();
	}
	return defaultValue;
}

//! @returns first keyboard, or null if not found
static InputDevicePtr getFirstKeyboard(const skybolt::InputPlatform& platform)
{
	auto keyboards = platform.getInputDevicesOfType(InputDeviceTypeKeyboard);
	return keyboards.empty() ? nullptr : keyboards.front();
}

void CameraInputSystem::advanceWallTime(sim::SecondsD newTime, sim::SecondsD dt)
{
	mDtWallClock += dt;
}

void CameraInputSystem::generateCameraInput()
{
	sim::SecondsD dt = 0;
	std::swap(mDtWallClock, dt);

	InputDevicePtr keyboard = getFirstKeyboard(*mInputPlatform);

	mInput.forwardSpeed = getAxisValueOrDefault(mAxes, CameraInputAxisType::Forward, 0.f);
	mInput.rightSpeed = getAxisValueOrDefault(mAxes, CameraInputAxisType::Right, 0.f);
	
	if (dt > 0)
	{
		mInput.yawRate /= dt;
		mInput.tiltRate /= dt;
		mInput.zoomRate /= dt;
	}
	else
	{
		mInput.yawRate = 0;
		mInput.tiltRate = 0;
		mInput.zoomRate = 0;
	}

	mInput.modifier1Pressed = keyboard ? keyboard->isButtonPressed(KC_LSHIFT) : false;
	mInput.modifier2Pressed = keyboard ? keyboard->isButtonPressed(KC_LCONTROL) : false;

	cameraInputGenerated(mInput);

	mInput = sim::CameraController::Input::zero();
}

void CameraInputSystem::onEvent(const Event &evt)
{
	if (const auto& event = dynamic_cast<const MouseEvent*>(&evt))
	{
		mInput.yawRate += event->relState.x * mCameraRotationAnglePerMousePixel;
		mInput.tiltRate -= event->relState.y * mCameraRotationAnglePerMousePixel;
		mInput.zoomRate += event->relState.z * 0.001;
	}
}

CameraInputAxes createDefaultCameraInputAxes(const skybolt::InputPlatform& inputPlatform)
{
	if (inputPlatform.getInputDevicesOfType(InputDeviceTypeKeyboard).empty())
	{
		BOOST_LOG_TRIVIAL(warning) << "Keyboard not found. Keyboard input will be ignored.'";
		return {};
	}

	InputDevicePtr keyboard = inputPlatform.getInputDevicesOfType(InputDeviceTypeKeyboard)[0];
	float rate = 1000;

	return {
		{ CameraInputAxisType::Forward, std::make_shared<KeyAxis>(keyboard, KC_S, KC_W, rate, rate, -1.0f, 1.0f) },
		{ CameraInputAxisType::Right, std::make_shared<KeyAxis>(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f) }
	};
}

void connectToCamera(CameraInputSystem& system, const sim::EntityPtr& camera)
{
	system.cameraInputGenerated.connect([camera] (const sim::CameraController::Input& input) {
		if (auto controller = camera->getFirstComponent<sim::CameraControllerComponent>(); controller)
		{
			if (controller->getSelectedController())
			{
				controller->getSelectedController()->setInput(input);
			}
		}
	});
}

void configure(CameraInputSystem& system, int screenHeightPixels, const nlohmann::json& engineSettings)
{
	float mouseSensitivity = engineSettings.at("mouse").at("sensitivity");
	if (mouseSensitivity <= 0)
	{
		BOOST_LOG_TRIVIAL(warning) << "Invalid mouse sensitivity value '" << mouseSensitivity << "'. Defaulting to 1.";
		mouseSensitivity = 1.f;
	}

	float rate = mouseSensitivity / screenHeightPixels;
	system.setCameraRotationAnglePerMousePixel(rate);
}

} // namespace skybolt