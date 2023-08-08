#include "ViewportInput.h"
#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltEngine/Input/LogicalAxis.h>

using namespace skybolt;

ViewportInput::ViewportInput(const InputPlatformPtr& inputPlatform) :
	mInputPlatform(inputPlatform)
{
	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);

	std::vector<InputDevicePtr> keyboards = inputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard);
	if (keyboards.empty())
	{
		throw std::runtime_error("Keyboard input device not found");
	}

	InputDevicePtr keyboard = keyboards.front();
	float rate = 1000;
	mForwardAxis = std::make_shared<KeyAxis>(keyboard, KC_S, KC_W, rate, rate, -1.0f, 1.0f);
	mLogicalAxes.push_back(mForwardAxis);

	mRightAxis = std::make_shared<KeyAxis>(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f);
	mLogicalAxes.push_back(mRightAxis);

	setEnabled(false);
}

ViewportInput::~ViewportInput()
{
	mInputPlatform->getEventEmitter()->removeEventListener(this);
}

void ViewportInput::setEnabled(bool enabled)
{
	for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeMouse))
	{
		device->setEnabled(enabled);
	}

	for (InputDevicePtr device : mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard))
	{
		device->setEnabled(enabled);
	}
	mEnabled = enabled;
}

void ViewportInput::updateBeforeInput()
{
	mInput = sim::CameraController::Input::zero();
}

void ViewportInput::updateAfterInput(float dt)
{
	for (const LogicalAxisPtr& device : mLogicalAxes)
	{
		device->update(dt);
	}
	mInput.forwardSpeed = mForwardAxis->getState();
	mInput.rightSpeed = mRightAxis->getState();
	mInput.panSpeed /= dt;
	mInput.tiltSpeed /= dt;
	mInput.zoomSpeed /= dt;
	mInput.modifier1Pressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LSHIFT);
	mInput.modifier2Pressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LCONTROL);
}

sim::CameraController::Input ViewportInput::getInput() const
{
	return mInput;
}

void ViewportInput::onEvent(const Event &evt)
{
	if (!mEnabled)
	{
		return;
	}

	if (const auto& event = dynamic_cast<const MouseEvent*>(&evt))
	{
		mInput.panSpeed += event->relState.x;
		mInput.tiltSpeed -= event->relState.y;
		mInput.zoomSpeed = event->relState.z;
	}
}
