/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InputPlatformOis.h"

#include <ois/OISMouse.h>
#include <ois/OISKeyboard.h>
#include <ois/OISJoyStick.h>
#include <ois/OISInputManager.h>
#include <ois/OISException.h>

#include <assert.h>
#include <boost/foreach.hpp>

namespace skybolt {

using InputManagerPtr = std::shared_ptr<OIS::InputManager>;

class KeyboardInputDevice : public InputDevice, public OIS::KeyListener
{
public:
	KeyboardInputDevice(const EventEmitterPtr& emitter, const InputManagerPtr& inputManager) :
		mEmitter(emitter),
		mInputManager(inputManager),
		mKeyboard(nullptr)
	{
		assert(mEmitter);
		setEnabled(true);
	}

	~KeyboardInputDevice()
	{
		setEnabled(false);
	}

	void setEnabled(bool enabled) override
	{
		if (!mKeyboard && enabled)
		{
			mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
			mKeyboard->setEventCallback(this);
			mKeyboard->setBuffered(true);
		}
		else if (mKeyboard && !enabled)
		{
			mInputManager->destroyInputObject(mKeyboard);
			mKeyboard = nullptr;
		}
	}

	virtual size_t getButtonCount() const { return (size_t)KC_ENUM_COUNT; }
	virtual bool isButtonPressed(size_t i) const
	{
		if (mKeyboard)
			return mKeyboard->isKeyDown((OIS::KeyCode)i);
		return false;
	}

	virtual size_t getAxisCount() const { return 0; }
	virtual float getAxisState(size_t i) const { assert(0); return 0.0f; }

	virtual const std::string& getName() const
	{
		if (mKeyboard)
			return mKeyboard->vendor();
		static std::string empty = "";
		return empty;
	}

	void capture()
	{
		if (mKeyboard)
			mKeyboard->capture();
	}

private:
	bool keyPressed(const OIS::KeyEvent &arg)
	{
		KeyEvent event(KeyEvent::Type::Pressed, (KeyCode)arg.key);
		mEmitter->emitEvent(event);
		return true;
	}

	bool keyReleased(const OIS::KeyEvent &arg)
	{
		KeyEvent event(KeyEvent::Type::Released, (KeyCode)arg.key);
		mEmitter->emitEvent(event);
		return true;
	}

private:
	EventEmitterPtr mEmitter;
	InputManagerPtr mInputManager;
	OIS::Keyboard* mKeyboard;
};

class MouseInputDevice : public InputDevice, public OIS::MouseListener
{
public:
	MouseInputDevice(const EventEmitterPtr& emitter, const InputManagerPtr& inputManager, int windowWidth, int windowHeight) :
		mEmitter(emitter),
		mInputManager(inputManager),
		mMouse(nullptr),
		mWindowWidth(windowWidth),
		mWindowHeight(windowHeight)
	{
		assert(mEmitter);
		setEnabled(true);
	}

	~MouseInputDevice()
	{
		setEnabled(false);
	}

	void setEnabled(bool enabled) override
	{
		if (!mMouse && enabled)
		{
			mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
			mMouse->setEventCallback(this);
			mMouse->setBuffered(true);
			const OIS::MouseState &s = mMouse->getMouseState();
			s.width = mWindowWidth;
			s.height = mWindowHeight;
		}
		else if (mMouse && !enabled)
		{
			mInputManager->destroyInputObject(mMouse);
			mMouse = nullptr;
		}
	}

	virtual size_t getButtonCount() const { return OIS::MB_Button7 + 1; }
	virtual bool isButtonPressed(size_t i) const
	{
		if (mMouse)
			return mMouse->getMouseState().buttonDown((OIS::MouseButtonID)i);
		return false;
	}

	virtual size_t getAxisCount() const { return 0; }
	virtual float getAxisState(size_t i) const { assert(0); return 0.0f; }

	virtual const std::string& getName() const
	{
		if (mMouse)
			return mMouse->vendor();
		static std::string empty = "";
		return empty;
	}

	void capture()
	{
		if (mMouse)
			mMouse->capture();
	}

	void setWindowWidth(int width)
	{
		mWindowWidth = width;
		if (mMouse)
		{
			const OIS::MouseState &s = mMouse->getMouseState();
			s.width = width;
		}
	}

	void setWindowHeight(int height)
	{
		mWindowHeight = height;
		if (mMouse)
		{
			const OIS::MouseState &s = mMouse->getMouseState();
			s.height = height;
		}
	}

private:
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
	{
		MouseEvent event;
		event.type = MouseEvent::Type::Pressed;
		event.buttonId = (MouseEvent::ButtonId)id;
		event.absState.x = (float)arg.state.X.abs;
		event.absState.y = (float)arg.state.Y.abs;
		event.absState.z = (float)arg.state.Z.abs;
		event.relState.x = (float)arg.state.X.rel;
		event.relState.y = (float)arg.state.Y.rel;
		event.relState.z = (float)arg.state.Z.rel;
		mEmitter->emitEvent(event);
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
	{
		MouseEvent event;
		event.type = MouseEvent::Type::Released;
		event.buttonId = (MouseEvent::ButtonId)id;
		event.absState.x = (float)arg.state.X.abs;
		event.absState.y = (float)arg.state.Y.abs;
		event.absState.z = (float)arg.state.Z.abs;
		event.relState.x = (float)arg.state.X.rel;
		event.relState.y = (float)arg.state.Y.rel;
		event.relState.z = (float)arg.state.Z.rel;
		mEmitter->emitEvent(event);
		return true;
	}

	bool mouseMoved(const OIS::MouseEvent &arg)
	{
		MouseEvent event;
		event.type = MouseEvent::Type::Moved;
		event.buttonId = MouseEvent::ButtonId::Left;
		event.absState.x = (float)arg.state.X.abs;
		event.absState.y = (float)arg.state.Y.abs;
		event.absState.z = (float)arg.state.Z.abs;
		event.relState.x = (float)arg.state.X.rel;
		event.relState.y = (float)arg.state.Y.rel;
		event.relState.z = (float)arg.state.Z.rel;
		mEmitter->emitEvent(event);
		return true;
	}

private:
	EventEmitterPtr mEmitter;
	InputManagerPtr mInputManager;
	OIS::Mouse* mMouse;
	int mWindowWidth;
	int mWindowHeight;
	bool destroyOnNextUpdate;
};

class JoystickInputDevice : public InputDevice
{
public:
	JoystickInputDevice(const InputManagerPtr& inputManager, const std::string& vendor) :
		mInputManager(inputManager),
		mJoystick(nullptr),
		mVendor(vendor)
	{
		setEnabled(true);
	}

	~JoystickInputDevice()
	{
		setEnabled(false);
	}

	void setEnabled(bool enabled) override
	{
		if (!mJoystick && enabled)
		{
			mJoystick = static_cast<OIS::JoyStick*>(mInputManager->createInputObject(OIS::OISJoyStick, true, mVendor));
		}
		else if (mJoystick && !enabled)
		{
			mInputManager->destroyInputObject(mJoystick);
			mJoystick = nullptr;
		}
	}

	virtual size_t getButtonCount() const
	{
		return mJoystick->getNumberOfComponents(OIS::OIS_Button);
	}

	virtual bool isButtonPressed(size_t i) const
	{
		if (mJoystick)
			return mJoystick->getJoyStickState().mButtons[i];
		return false;
	}

	virtual size_t getAxisCount() const
	{
		return mJoystick->getNumberOfComponents(OIS::OIS_Axis);
	}

	virtual float getAxisState(size_t i) const
	{
		assert(i < getAxisCount());

		if (mJoystick)
		{
			return 0.5f + (float)mJoystick->getJoyStickState().mAxes.at(i).abs / 65535.0f;
		}
		return 0.5f;
	}

	virtual const std::string& getName() const
	{
		return mVendor;
	}

	void capture()
	{
		if (mJoystick)
			mJoystick->capture();
	}

private:
	InputManagerPtr mInputManager;
	OIS::JoyStick* mJoystick;
	std::string mVendor;
};

InputPlatformOis::InputPlatformOis(const std::string &windowHandle, int windowWidth, int windowHeight) :
	mEmitter(std::make_shared<EventEmitter>())
{
	OIS::ParamList pl;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHandle));
	mInputManager = std::shared_ptr<OIS::InputManager>(OIS::InputManager::createInputSystem(pl), [](OIS::InputManager* mgr) {
		OIS::InputManager::destroyInputSystem(mgr);
	});

	mKeyboard.reset(new KeyboardInputDevice(mEmitter, mInputManager));
	mMouse.reset(new MouseInputDevice(mEmitter, mInputManager, windowWidth, windowHeight));

	const OIS::DeviceList& deviceList = mInputManager->listFreeDevices();
	for (OIS::DeviceList::const_iterator i = deviceList.find(OIS::OISJoyStick); i != deviceList.end(); ++i)
	{
		const std::string& vendor = i->second;
		std::shared_ptr<JoystickInputDevice> joystick;
		try
		{
			joystick.reset(new JoystickInputDevice(mInputManager, vendor));
		}
		catch (const OIS::Exception&)
		{
		}
		if (joystick)
			mJoysticks.push_back(joystick);
	}
}

InputPlatformOis::~InputPlatformOis()
{
	mKeyboard.reset();
	mMouse.reset();
	mJoysticks.clear();
}


void InputPlatformOis::update()
{
	mMouse->capture();
	mKeyboard->capture();

	for (const std::shared_ptr<JoystickInputDevice>& joystick : mJoysticks)
		joystick->capture();
}

void InputPlatformOis::setWindowWidth(int width)
{
	mMouse->setWindowWidth(width);
}

void InputPlatformOis::setWindowHeight(int height)
{
	mMouse->setWindowHeight(height);
}

std::vector<InputDevicePtr> InputPlatformOis::getInputDevicesOfType(InputDeviceType type) const
{
	std::vector<InputDevicePtr> devices;
	switch (type)
	{
	case InputDeviceTypeKeyboard:
		devices.push_back(mKeyboard);
		break;
	case InputDeviceTypeMouse:
		devices.push_back(mMouse);
		break;
	case InputDeviceTypeJoystick:
		devices.insert(devices.end(), mJoysticks.begin(), mJoysticks.end());
		break;
	default:
		assert("!Unknown device type");
	}
	return devices;
}

void InputPlatformOis::setEnabled(bool enabled)
{
	mKeyboard->setEnabled(enabled);
	mMouse->setEnabled(enabled);

	for (const std::shared_ptr<JoystickInputDevice>& joystick : mJoysticks)
		joystick->setEnabled(enabled);
}

} // namespace skybolt