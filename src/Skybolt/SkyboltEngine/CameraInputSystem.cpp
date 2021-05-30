/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraInputSystem.h"
#include "Input/InputPlatform.h"
#include "Input/LogicalAxis.h"
#include "SimVisBinding/CameraSimVisBinding.h"
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Window/Window.h>

namespace skybolt {

using namespace sim;

CameraInputSystem::CameraInputSystem(const EntityPtr& camera, const InputPlatformPtr& inputPlatform, const std::vector<LogicalAxisPtr>& axes) :
	mCamera(camera),
	mInputPlatform(inputPlatform),
	mInputAxes(axes),
	mInput(CameraController::Input::zero())
{
	assert(mEnabled);
	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);
}

CameraInputSystem::~CameraInputSystem()
{
	mInputPlatform->getEventEmitter()->removeEventListener(this);
}

void CameraInputSystem::updatePostDynamics(const System::StepArgs& args)
{
	if (!mEnabled)
	{
		return;
	}

	if (mInputAxes.size() == 2)
	{
		mInput.forwardSpeed = mInputAxes[0]->getState();
		mInput.rightSpeed = mInputAxes[1]->getState();
	}

	mInput.panSpeed /= args.dtWallClock;
	mInput.tiltSpeed /= args.dtWallClock;
	mInput.zoomSpeed /= args.dtWallClock;
	mInput.modifierPressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LSHIFT);

	auto cameraControllerComponent = mCamera->getFirstComponent<sim::CameraControllerComponent>();
	if (cameraControllerComponent)
	{
		cameraControllerComponent->cameraController->setInput(mInput);
	}

	mInput = CameraController::Input::zero();

	if (cameraControllerComponent)
	{
		cameraControllerComponent->cameraController->update(args.dtWallClock);
	}
}

void CameraInputSystem::setInputEnabled(bool enabled)
{
	if (enabled && !mEnabled)
	{
		mInput = CameraController::Input::zero();
		mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);
	}
	else if (!enabled && mEnabled)
	{
		mInputPlatform->getEventEmitter()->removeEventListener(this);
	}
	mEnabled = enabled;
}

void CameraInputSystem::onEvent(const Event &event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		if (mouseEvent->type == MouseEvent::Type::Moved)
		{
			mInput.panSpeed += mouseEvent->relState.x;
			mInput.tiltSpeed -= mouseEvent->relState.y;
			mInput.zoomSpeed = mouseEvent->relState.z;
		}
	}
}

std::vector<LogicalAxisPtr> CameraInputSystem::createDefaultAxes(const InputPlatform& inputPlatform)
{
	InputDevicePtr keyboard = inputPlatform.getInputDevicesOfType(InputDeviceTypeKeyboard)[0];
	float rate = 1000;

	std::vector<LogicalAxisPtr> logicalAxes;

	LogicalAxisPtr forwardAxis(new KeyAxis(keyboard, KC_S, KC_W, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(forwardAxis);

	LogicalAxisPtr rightAxis(new KeyAxis(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(rightAxis);

	return logicalAxes;
}

} // skybolt