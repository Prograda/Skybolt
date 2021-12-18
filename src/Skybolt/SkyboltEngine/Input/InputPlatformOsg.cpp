/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InputPlatformOsg.h"
#include <osgViewer/Viewer>

#include <assert.h>
#include <optional>

namespace skybolt {

static std::optional<KeyCode> osgToOisKeyCode(osgGA::GUIEventAdapter::KeySymbol symbol)
{
	using Adapter = osgGA::GUIEventAdapter;
	switch (symbol)
	{
		case Adapter::KEY_Space: return KC_SPACE;
		case Adapter::KEY_0: return KC_0;
		case Adapter::KEY_1: return KC_1;
		case Adapter::KEY_2: return KC_2;
		case Adapter::KEY_3: return KC_3;
		case Adapter::KEY_4: return KC_4;
		case Adapter::KEY_5: return KC_5;
		case Adapter::KEY_6: return KC_6;
		case Adapter::KEY_7: return KC_7;
		case Adapter::KEY_8: return KC_8;
		case Adapter::KEY_9: return KC_9;
		case Adapter::KEY_A: return KC_A;
		case Adapter::KEY_B: return KC_B;
		case Adapter::KEY_C: return KC_C;
		case Adapter::KEY_D: return KC_D;
		case Adapter::KEY_E: return KC_E;
		case Adapter::KEY_F: return KC_F;
		case Adapter::KEY_G: return KC_G;
		case Adapter::KEY_H: return KC_H;
		case Adapter::KEY_I: return KC_I;
		case Adapter::KEY_J: return KC_J;
		case Adapter::KEY_K: return KC_K;
		case Adapter::KEY_L: return KC_L;
		case Adapter::KEY_M: return KC_M;
		case Adapter::KEY_N: return KC_N;
		case Adapter::KEY_O: return KC_O;
		case Adapter::KEY_P: return KC_P;
		case Adapter::KEY_Q: return KC_Q;
		case Adapter::KEY_R: return KC_R;
		case Adapter::KEY_S: return KC_S;
		case Adapter::KEY_T: return KC_T;
		case Adapter::KEY_U: return KC_U;
		case Adapter::KEY_V: return KC_V;
		case Adapter::KEY_W: return KC_W;
		case Adapter::KEY_X: return KC_X;
		case Adapter::KEY_Y: return KC_Y;
		case Adapter::KEY_Z: return KC_Z;
		//case Adapter::KEY_Exclaim:
		//case Adapter::KEY_Quotedbl
		//case Adapter::KEY_Hash
		//case Adapter::KEY_Dollar
		//case Adapter::KEY_Ampersand
		//case Adapter::KEY_Quote
		//case Adapter::KEY_Leftparen
		//case Adapter::KEY_Rightparen
		//case Adapter::KEY_Asterisk
		//case Adapter::KEY_Plus
		case Adapter::KEY_Comma: return KC_COMMA;
		case Adapter::KEY_Minus: return KC_MINUS;
		case Adapter::KEY_Period: return KC_PERIOD;
		case Adapter::KEY_Slash: return KC_SLASH;
		case Adapter::KEY_Colon: return KC_COLON;
		case Adapter::KEY_Semicolon: return KC_SEMICOLON;
		//case Adapter::KEY_Less
		case Adapter::KEY_Equals: return KC_EQUALS;
		//case Adapter::KEY_Greater
		//case Adapter::KEY_Question
		//case Adapter::KEY_At
		case Adapter::KEY_Leftbracket: return KC_LBRACKET;
		case Adapter::KEY_Backslash: return KC_BACKSLASH;
		case Adapter::KEY_Rightbracket: return KC_RBRACKET;
		//se Adapter::KEY_Caret
		case Adapter::KEY_Underscore: return KC_UNDERLINE;
		//case Adapter::KEY_Backquote
		case Adapter::KEY_BackSpace: return KC_BACK;
		case Adapter::KEY_Tab: return KC_TAB;
		//case Adapter::KEY_Linefeed
		//case Adapter::KEY_Clear
		case Adapter::KEY_Return: return KC_RETURN;
		case Adapter::KEY_Pause: return KC_PAUSE;
		case Adapter::KEY_Scroll_Lock: return KC_SCROLL;
		case Adapter::KEY_Sys_Req: return KC_SYSRQ;
		case Adapter::KEY_Escape: return KC_ESCAPE;
		case Adapter::KEY_Delete: return KC_DELETE;
		case Adapter::KEY_Home: return KC_HOME;
		case Adapter::KEY_Left: return KC_LEFT;
		case Adapter::KEY_Up: return KC_UP;
		case Adapter::KEY_Right: return KC_RIGHT;
		case Adapter::KEY_Down: return KC_DOWN;
		//case Adapter::KEY_Prior
		case Adapter::KEY_Page_Up: return KC_PGUP;
		case Adapter::KEY_Page_Down: return KC_PGDOWN;
		case Adapter::KEY_End: return KC_END;
		case Adapter::KEY_Begin: return KC_HOME;
		//case Adapter::KEY_Select
		//case Adapter::KEY_Print
		//case Adapter::KEY_Execute
		case Adapter::KEY_Insert: return KC_INSERT;
		//case Adapter::KEY_Undo
		//case Adapter::KEY_Redo
		case Adapter::KEY_Menu: return KC_LMENU;
		//case Adapter::KEY_Find
		//case Adapter::KEY_Cancel
		//case Adapter::KEY_Help
		case Adapter::KEY_Break: return KC_PAUSE;
		//case Adapter::KEY_Mode_switch
		//case Adapter::KEY_Script_switch
		case Adapter::KEY_Num_Lock: return KC_NUMLOCK;
		//case Adapter::KEY_KP_Space
		//case Adapter::KEY_KP_Tab
		case Adapter::KEY_KP_Enter: return KC_NUMPADENTER;
		//case Adapter::KEY_KP_F1
		//case Adapter::KEY_KP_F2
		//case Adapter::KEY_KP_F3
		//case Adapter::KEY_KP_F4
		//case Adapter::KEY_KP_Home
		//case Adapter::KEY_KP_Left
		//case Adapter::KEY_KP_Up
		//case Adapter::KEY_KP_Right
		//case Adapter::KEY_KP_Down
		//case Adapter::KEY_KP_Prior
		//case Adapter::KEY_KP_Page_Up
		//case Adapter::KEY_KP_Next
		//case Adapter::KEY_KP_Page_Down
		//case Adapter::KEY_KP_End
		//case Adapter::KEY_KP_Begin
		//case Adapter::KEY_KP_Insert
		//case Adapter::KEY_KP_Delete
		case Adapter::KEY_KP_Equal: return KC_EQUALS;
		//case Adapter::KEY_KP_Multiply
		case Adapter::KEY_KP_Add: return KC_NUMPADEQUALS;
		//case Adapter::KEY_KP_Separator
		//case Adapter::KEY_KP_Subtract
		case Adapter::KEY_KP_Decimal: return KC_NUMPADCOMMA;
		case Adapter::KEY_KP_0: return KC_NUMPAD0;
		case Adapter::KEY_KP_1: return KC_NUMPAD1;
		case Adapter::KEY_KP_2: return KC_NUMPAD2;
		case Adapter::KEY_KP_3: return KC_NUMPAD3;
		case Adapter::KEY_KP_4: return KC_NUMPAD4;
		case Adapter::KEY_KP_5: return KC_NUMPAD5;
		case Adapter::KEY_KP_6: return KC_NUMPAD6;
		case Adapter::KEY_KP_7: return KC_NUMPAD7;
		case Adapter::KEY_KP_8: return KC_NUMPAD8;
		case Adapter::KEY_KP_9: return KC_NUMPAD9;
		case Adapter::KEY_F1: return KC_F1;
		case Adapter::KEY_F2: return KC_F2;
		case Adapter::KEY_F3: return KC_F3;
		case Adapter::KEY_F4: return KC_F4;
		case Adapter::KEY_F5: return KC_F5;
		case Adapter::KEY_F6: return KC_F6;
		case Adapter::KEY_F7: return KC_F7;
		case Adapter::KEY_F8: return KC_F8;
		case Adapter::KEY_F9: return KC_F9;
		case Adapter::KEY_F10: return KC_F10;
		case Adapter::KEY_F11: return KC_F11;
		case Adapter::KEY_F12: return KC_F12;
		case Adapter::KEY_F13: return KC_F13;
		case Adapter::KEY_F14: return KC_F14;
		case Adapter::KEY_F15: return KC_F15;
		//case Adapter::KEY_F16
		//case Adapter::KEY_F17
		//case Adapter::KEY_F18
		//case Adapter::KEY_F19
		//case Adapter::KEY_F20
		//case Adapter::KEY_F21
		//case Adapter::KEY_F22
		//case Adapter::KEY_F23
		//case Adapter::KEY_F24
		//case Adapter::KEY_F25
		//case Adapter::KEY_F26
		//case Adapter::KEY_F27
		//case Adapter::KEY_F28
		//case Adapter::KEY_F29
		//case Adapter::KEY_F30
		//case Adapter::KEY_F31
		//case Adapter::KEY_F32
		//case Adapter::KEY_F33
		//case Adapter::KEY_F34
		//case Adapter::KEY_F35
		case Adapter::KEY_Shift_L: return KC_LSHIFT;
		case Adapter::KEY_Shift_R: return KC_RSHIFT;
		case Adapter::KEY_Control_L: return KC_LCONTROL;
		case Adapter::KEY_Control_R: return KC_RCONTROL;
		case Adapter::KEY_Caps_Lock: return KC_CAPITAL;
		//case Adapter::KEY_Shift_Lock
		//case Adapter::KEY_Meta_L
		//case Adapter::KEY_Meta_R
		case Adapter::KEY_Alt_L: return KC_LMENU;
		case Adapter::KEY_Alt_R: return KC_RMENU;
		//case Adapter::KEY_Super_L
		//case Adapter::KEY_Super_R
		//case Adapter::KEY_Hyper_L
		//case Adapter::KEY_Hyper_R
	}
	return {};
}

// Forward events to a templated class.
// Required because OSG event handlers must be stored in osg::ref_ptr,
// But we want our event handlers to be an std::shared_ptr.
template <class EventHandlerT>
struct EventHandlerFwd : public osgGA::GUIEventHandler
{
	EventHandlerFwd(const std::shared_ptr<EventHandlerT>& handler) : handler(handler) {}

	bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override
	{
		return handler->handle(ea, aa);
	}
private:
	std::shared_ptr<EventHandlerT> handler;
};

class KeyboardInputDeviceOsg : public InputDevice
{
public:
	KeyboardInputDeviceOsg(const EventEmitterPtr& emitter) :
		mEmitter(emitter)
	{
		assert(mEmitter);
		setEnabled(true);
	}

	~KeyboardInputDeviceOsg()
	{
		setEnabled(false);
	}

	void setEnabled(bool enabled) override
	{
		mEnabled = enabled;
		if (!mEnabled)
		{
			mPressedKeys.clear();
		}
	}

	virtual size_t getButtonCount() const { return (size_t)KC_ENUM_COUNT; }
	virtual bool isButtonPressed(size_t i) const
	{
		return mEnabled && mPressedKeys.find((KeyCode)i) != mPressedKeys.end();
	}

	virtual size_t getAxisCount() const { return 0; }
	virtual float getAxisState(size_t i) const { assert(0); return 0.0f; }

	virtual const std::string& getName() const
	{
		static std::string keyboard = "Keyboard";
		return keyboard;
	}

	bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
	{
		if (mEnabled)
		{
			switch (ea.getEventType())
			{
				case osgGA::GUIEventAdapter::KEYDOWN:
				{
					auto keyCode = osgToOisKeyCode((osgGA::GUIEventAdapter::KeySymbol)ea.getKey());
					if (keyCode)
					{
						mPressedKeys.insert(*keyCode);
						KeyEvent event(KeyEvent::Type::Pressed, *keyCode);
						mEmitter->emitEvent(event);
						return true;
					}
				}
				case osgGA::GUIEventAdapter::KEYUP:
				{
					auto keyCode = osgToOisKeyCode((osgGA::GUIEventAdapter::KeySymbol)ea.getKey());
					if (keyCode)
					{
						mPressedKeys.erase(*keyCode);
						KeyEvent event(KeyEvent::Type::Released, *keyCode);
						mEmitter->emitEvent(event);
						return true;
					}
				}
			}
		}
		return false;
	}

private:
	EventEmitterPtr mEmitter;
	std::set<KeyCode> mPressedKeys;
	bool mEnabled = true;
};

MouseInputDeviceOsg::MouseInputDeviceOsg(const EventEmitterPtr& emitter) :
	mEmitter(emitter)
{
	assert(mEmitter);
	setEnabled(true);
}

MouseInputDeviceOsg::~MouseInputDeviceOsg()
{
	setEnabled(false);
}

void MouseInputDeviceOsg::setEnabled(bool enabled)
{
	if (mEnabled != enabled)
	{
		mEnabled = enabled;
		if (!mEnabled)
		{
			mPressedButtons.clear();
		}
		else
		{
			mPrevPosition.reset();
		}
	}
}

bool MouseInputDeviceOsg::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	if (mEnabled)
	{
		switch (ea.getEventType())
		{
			case osgGA::GUIEventAdapter::PUSH:
			{
				MouseEvent event;
				event.type = MouseEvent::Type::Pressed;
				event.buttonId = (MouseEvent::ButtonId)(ea.getButton() - 1);
				setMouseState(event, ea, aa);
				mEmitter->emitEvent(event);
				return true;
			}
			case osgGA::GUIEventAdapter::RELEASE:
			{
				MouseEvent event;
				event.type = MouseEvent::Type::Released;
				event.buttonId = (MouseEvent::ButtonId)(ea.getButton() - 1);
				setMouseState(event, ea, aa);
				mEmitter->emitEvent(event);
				return true;
			}
			case osgGA::GUIEventAdapter::MOVE:
			case osgGA::GUIEventAdapter::DRAG:
			{
				MouseEvent event;
				event.type = MouseEvent::Type::Moved;
				event.buttonId = MouseEvent::ButtonId::Left;
				setMouseState(event, ea, aa);
				mEmitter->emitEvent(event);
				return true;
			}
			case osgGA::GUIEventAdapter::SCROLL:
			{
				MouseEvent event;
				event.type = MouseEvent::Type::Moved;
				event.buttonId = MouseEvent::ButtonId::Left;
				setMouseState(event, ea, aa);
				mEmitter->emitEvent(event);
				return true;
			}
		}
	}
	return false;
}

void MouseInputDeviceOsg::setMouseState(MouseEvent& event, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	// OSG uses +Y upwards by default. Flip to +Y downwards coordinates.
	float flippedY = float(ea.getWindowHeight() - 1) - ea.getY();

	event.absState.x = ea.getX();
	event.absState.y = flippedY;
	event.absState.z = 0;
	event.relState.x = mPrevPosition ? std::floor(ea.getX() - mPrevPosition->x) : 0.0;
	event.relState.y = mPrevPosition  ? -std::floor(ea.getY() - mPrevPosition->y) : 0.0; // negate to flip from +Y upward to +Y downwards
	event.relState.z = ea.getScrollingDeltaY();

	if (mWrapEnabled)
	{
		mPrevPosition = glm::vec2(
			(ea.getXmin() + ea.getXmax()) / 2.0f,
			(ea.getYmin() + ea.getYmax()) / 2.0f);

		aa.requestWarpPointer(mPrevPosition->x, mPrevPosition->y);
	}
	else
	{
		mPrevPosition = glm::vec2(ea.getX(), ea.getY());
	}
}

void MouseInputDeviceOsg::enablePointerWrap(bool wrap)
{
	mWrapEnabled = wrap;
	mPrevPosition.reset();
}

InputPlatformOsg::InputPlatformOsg(const std::weak_ptr<osgViewer::Viewer>& viewer) :
	mViewer(viewer),
	mEmitter(std::make_shared<EventEmitter>())
{
	mKeyboard = std::make_shared<KeyboardInputDeviceOsg>(mEmitter);
	mMouse = std::make_shared<MouseInputDeviceOsg>(mEmitter);

	addEventHandler(osg::ref_ptr<osgGA::GUIEventHandler>(new EventHandlerFwd(mKeyboard)));
	addEventHandler(osg::ref_ptr<osgGA::GUIEventHandler>(new EventHandlerFwd(mMouse)));
}

void InputPlatformOsg::addEventHandler(const osg::ref_ptr<osgGA::GUIEventHandler>& handler)
{
	if (auto viewer = mViewer.lock(); viewer)
	{
		mEventHandlers.push_back(handler);
		viewer->addEventHandler(handler);
	}
}

InputPlatformOsg::~InputPlatformOsg()
{
	if (auto viewer = mViewer.lock(); viewer)
	{
		for (const auto& handler : mEventHandlers)
		{
			viewer->removeEventHandler(handler);
		}
	}
}

std::vector<InputDevicePtr> InputPlatformOsg::getInputDevicesOfType(InputDeviceType type) const
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
		break;
	default:
		assert("!Unknown device type");
	}
	return devices;
}

void InputPlatformOsg::setEnabled(bool enabled)
{
	mKeyboard->setEnabled(enabled);
	mMouse->setEnabled(enabled);
}

} // namespace skybolt