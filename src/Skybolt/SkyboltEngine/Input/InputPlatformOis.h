/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "InputPlatform.h"
#include "SkyboltCommon/Event.h"
#include "SkyboltCommon/Math/MathUtility.h"
#include <string>

namespace OIS
{
	class InputManager;
}

namespace skybolt {

struct KeyEvent : public Event
{
	enum Type
	{
		Pressed,
		Released
	};

	KeyEvent(Type type, KeyCode code) :
		type(type),
		code(code)
	{
	}

	Type type;
	KeyCode code;
};

struct MouseEvent : public Event
{
	enum Type
	{
		Pressed,
		Released,
		Moved
	};

	enum ButtonId
	{
		Left,
		Right,
		Middle,
		Button3,
		Button4,
		Button5,
		Button6,
		Button7
	};

	Type type;
	ButtonId buttonId;
	glm::vec3 absState;
	glm::vec3 relState;
};

class InputPlatformOis : public InputPlatform
{
public:
	//!@param windowHandle is the handle for the window. On windows this is the HWND number as a string.
	InputPlatformOis(const std::string &windowHandle, int windowWidth, int windowHeight);
	~InputPlatformOis() override;

	void update() override;

	void setEnabled(bool enabled) override;

	std::vector<InputDevicePtr> getInputDevicesOfType(InputDeviceType type) const override;

	virtual EventEmitterPtr getEventEmitter() const override { return mEmitter; }

	void setWindowWidth(int width);
	void setWindowHeight(int height);

private:
	EventEmitterPtr mEmitter;
	std::shared_ptr<OIS::InputManager> mInputManager;
	std::shared_ptr<class KeyboardInputDevice> mKeyboard;
	std::shared_ptr<class MouseInputDevice> mMouse;
	std::vector<std::shared_ptr<class JoystickInputDevice> > mJoysticks;
};

} // namespace skybolt