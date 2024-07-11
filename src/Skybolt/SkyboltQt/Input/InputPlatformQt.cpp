/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InputPlatformQt.h"
#include "QtKeyCodeMapping.h"

#include <QApplication>
#include <QDesktopWidget>

#include <assert.h>
#include <boost/foreach.hpp>
#include <optional>

namespace skybolt {

constexpr int wrapMargin = 10; //!< Mouse cursor wraps at edges of screen, inset by this margin value

struct KeyboardInputDevice : public InputDevice
{
	bool enabled = true;
	std::set<KeyCode> pressedKeys;

	void setEnabled(bool enabled) override
	{
		this->enabled = enabled;
		if (!enabled)
		{
			pressedKeys.clear();
		}
	}

	size_t getButtonCount() const override { return (size_t)KC_ENUM_COUNT; }
	bool isButtonPressed(size_t i) const override
	{
		return pressedKeys.find(KeyCode(i)) != pressedKeys.end();
	}

	size_t getAxisCount() const override { return 0; }
	float getAxisState(size_t i) const override { assert(0); return 0.0f; }

	const std::string& getName() const override
	{
		static std::string name = "DefaultKeyboard";
		return name;
	}
};

struct MouseInputDevice : public InputDevice
{
	bool enabled = true;
	std::set<MouseEvent::ButtonId> pressedButtons;
	std::optional<int> prevMousePosX;
	std::optional<int> prevMousePosY;
	std::optional<int> prevMousePosZ;

	void setEnabled(bool enabled) override
	{
		this->enabled = enabled;
		if (!enabled)
		{
			pressedButtons.clear();
			prevMousePosX = std::nullopt;
			prevMousePosY = std::nullopt;
			prevMousePosZ = std::nullopt;
		}
	}

	size_t getButtonCount() const override { return MouseEvent::ButtonId::ButtonCount; }
	bool isButtonPressed(size_t i) const
	{
		return pressedButtons.find(MouseEvent::ButtonId(i)) != pressedButtons.end();
	}

	size_t getAxisCount() const override { return 0; }
	float getAxisState(size_t i) const override { assert(0); return 0.0f; }

	const std::string& getName() const override
	{
		static std::string name = "DefaultMouse";
		return name;
	}
};

//! Returns the difference from a to b (b minus a), accounting for wrapping of the dimension over wrapSize
static int calcWrappedDifference(int valueA, int valueB, int wrapSize)
{
	int delta = valueB - valueA;
	if (delta > wrapSize / 2)
	{
		delta -= wrapSize;
	}
	else if (delta < -wrapSize / 2)
	{
		delta += wrapSize;
	}
	return delta;
}

class InputPlatformQtEventFilter: public QObject
{
public:
	InputPlatformQtEventFilter(EventEmitterPtr emitter, std::shared_ptr<KeyboardInputDevice> keyboard, std::shared_ptr<MouseInputDevice> mouse) :
		mEmitter(std::move(emitter)),
		mKeyboard(std::move(keyboard)),
		mMouse(std::move(mouse))
	{
		assert(mEmitter);
		assert(mKeyboard);
		assert(mMouse);
	}

protected:
	bool eventFilter(QObject* object, QEvent* event)
	{
		switch (event->type())
		{
			case QEvent::KeyPress:
			{
				auto keyEvent = static_cast<QKeyEvent*>(event);
				KeyCode code = qtToSkyboltKeyCode(Qt::Key(keyEvent->key()), keyEvent->modifiers());
				mKeyboard->pressedKeys.insert(code);

				if (mKeyboard->enabled)
				{
					KeyEvent event(KeyEvent::Type::Pressed, code);
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::KeyRelease:
			{
				auto keyEvent = static_cast<QKeyEvent*>(event);
				KeyCode code = qtToSkyboltKeyCode(Qt::Key(keyEvent->key()), keyEvent->modifiers());
				mKeyboard->pressedKeys.erase(code);

				if (mKeyboard->enabled)
				{
					KeyEvent event(KeyEvent::Type::Released, code);
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::MouseButtonPress:
			{
				auto mouseEvent = static_cast<QMouseEvent*>(event);
				mMouse->pressedButtons.insert((MouseEvent::ButtonId)mouseEvent->button());

				if (mMouse->enabled)
				{
					mMouse->prevMousePosX = mouseEvent->pos().x();
					mMouse->prevMousePosY = mouseEvent->pos().y();

					MouseEvent event;
					event.type = MouseEvent::Type::Pressed;
					event.buttonId = (MouseEvent::ButtonId)mouseEvent->button();
					event.absState.x = mouseEvent->pos().x();
					event.absState.y = mouseEvent->pos().y();
					event.absState.z = 0;
					event.relState.x = 0;
					event.relState.y = 0;
					event.relState.z = 0;
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::MouseButtonRelease:
			{
				auto mouseEvent = static_cast<QMouseEvent*>(event);
				mMouse->pressedButtons.erase((MouseEvent::ButtonId)mouseEvent->button());

				if (mMouse->enabled)
				{
					MouseEvent event;
					event.type = MouseEvent::Type::Released;
					event.buttonId = (MouseEvent::ButtonId)mouseEvent->button();
					event.absState.x = mouseEvent->pos().x();
					event.absState.y = mouseEvent->pos().y();
					event.absState.z = 0;
					event.relState.x = 0;
					event.relState.y = 0;
					event.relState.z = 0;
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::MouseMove:
			{
				if (mMouse->enabled)
				{
					auto mouseEvent = static_cast<QMouseEvent*>(event);

					// Calculate relative mouse movement
					if (!mMouse->prevMousePosX)
					{
						mMouse->prevMousePosX = mouseEvent->pos().x();
						mMouse->prevMousePosY = mouseEvent->pos().y();
					}

					QSize wrapDimensionsSize = QApplication::desktop()->size();
					QPoint wrapDimensions = QPoint(wrapDimensionsSize.width() - wrapMargin * 2, wrapDimensionsSize.height() - wrapMargin * 2);

					int relX = calcWrappedDifference(*mMouse->prevMousePosX, mouseEvent->pos().x(), wrapDimensions.x());
					int relY = calcWrappedDifference(*mMouse->prevMousePosY, mouseEvent->pos().y(), wrapDimensions.y());

					mMouse->prevMousePosX = mouseEvent->pos().x();
					mMouse->prevMousePosY = mouseEvent->pos().y();

					// Send event
					MouseEvent event;
					event.type = MouseEvent::Type::Moved;
					event.buttonId = (MouseEvent::ButtonId)mouseEvent->button();
					event.absState.x = mouseEvent->pos().x();
					event.absState.y = mouseEvent->pos().y();
					event.absState.z = 0;
					event.relState.x = relX;
					event.relState.y = relY;
					event.relState.z = 0;
					mEmitter->emitEvent(event);

					// Wrap mouse cursor at edges of screen
					QPoint point = mouseEvent->globalPos() + wrapDimensions - QPoint(wrapMargin, wrapMargin);
					point.setX((point.x() % wrapDimensions.x()) + wrapMargin);
					point.setY((point.y() % wrapDimensions.y()) + wrapMargin);

					if (point != mouseEvent->globalPos())
					{
						QCursor::setPos(point);
					}

					return true;
				}
				break;
			}
			case QEvent::Wheel:
			{
				if (mMouse->enabled)
				{
					auto wheelEvent = static_cast<QWheelEvent*>(event);

					MouseEvent event;
					event.type = MouseEvent::Type::Moved;
					event.buttonId = MouseEvent::ButtonId::Left;
					event.absState.x = 0;
					event.absState.y = 0;
					event.absState.z = 0;
					event.relState.x = 0;
					event.relState.y = 0;
					event.relState.z = wheelEvent->angleDelta().y();
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::TouchBegin: {
				if (mMouse->enabled)
				{
					auto touchEvent = dynamic_cast<QTouchEvent*>(event);
					QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();

					if (points.empty())
						break;
					QTouchEvent::TouchPoint point = points[0];

					mMouse->pressedButtons.insert((MouseEvent::ButtonId)Qt::LeftButton);

					mMouse->prevMousePosX = point.pos().x();
					mMouse->prevMousePosY = point.pos().y();

					MouseEvent event;
					event.type = MouseEvent::Type::Pressed;
					event.buttonId = (MouseEvent::ButtonId)Qt::LeftButton;
					event.absState.x = point.pos().x();
					event.absState.y = point.pos().y();
					event.absState.z = 0;
					event.relState.x = 0;
					event.relState.y = 0;
					event.relState.z = 0;
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}
			case QEvent::TouchEnd: {
				if (mMouse->enabled)
				{
					auto touchEvent = dynamic_cast<QTouchEvent*>(event);
					QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
					if (points.empty())
						break;
					QTouchEvent::TouchPoint point = points[0];

					mMouse->pressedButtons.insert((MouseEvent::ButtonId)Qt::LeftButton);

					mMouse->prevMousePosX = point.pos().x();
					mMouse->prevMousePosY = point.pos().y();

					MouseEvent event;
					event.type = MouseEvent::Type::Released;
					event.buttonId = (MouseEvent::ButtonId)Qt::LeftButton;
					event.absState.x = point.pos().x();
					event.absState.y = point.pos().y();
					event.absState.z = 0;
					event.relState.x = 0;
					event.relState.y = 0;
					event.relState.z = 0;
					mEmitter->emitEvent(event);
					return true;
				}
				break;
			}

			case QEvent::TouchUpdate: {
				if (mMouse->enabled)
				{
					auto touchEvent = dynamic_cast<QTouchEvent*>(event);
					QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
					if (points.empty())
						break;

					if (points.size() == 1)
					{
						QTouchEvent::TouchPoint point = points[0];

						// Calculate relative mouse movement
						if (!mMouse->prevMousePosX)
						{
							mMouse->prevMousePosX = point.pos().x();
							mMouse->prevMousePosY = point.pos().y();
						}

						QSize wrapDimensionsSize = QApplication::desktop()->size();
						QPoint wrapDimensions = QPoint(wrapDimensionsSize.width() - wrapMargin * 2, wrapDimensionsSize.height() - wrapMargin * 2);

						int relX = calcWrappedDifference(*mMouse->prevMousePosX, point.pos().x(), wrapDimensions.x());
						int relY = calcWrappedDifference(*mMouse->prevMousePosY, point.pos().y(), wrapDimensions.y());

						mMouse->prevMousePosX = point.pos().x();
						mMouse->prevMousePosY = point.pos().y();

						MouseEvent event;
						event.type = MouseEvent::Type::Moved;
						event.buttonId = (MouseEvent::ButtonId)Qt::LeftButton;
						event.absState.x = point.pos().x();
						event.absState.y = point.pos().y();
						event.absState.z = 0;
						event.relState.x = relX;
						event.relState.y = relY;
						event.relState.z = 0;
						mEmitter->emitEvent(event);
						return true;
					}
					else // multi touch zoom
					{
						double prevLength = (points[1].lastPos() - points[0].lastPos()).manhattanLength();
						double newLength = (points[1].pos() - points[0].pos()).manhattanLength();

						double wheelAngle = (newLength - prevLength);

						if (mMouse->enabled)
						{
							auto wheelEvent = static_cast<QWheelEvent*>(event);

							MouseEvent event;
							event.type = MouseEvent::Type::Moved;
							event.buttonId = MouseEvent::ButtonId::Left;
							event.absState.x = 0;
							event.absState.y = 0;
							event.absState.z = 0;
							event.relState.x = 0;
							event.relState.y = 0;
							event.relState.z = wheelAngle;
							mEmitter->emitEvent(event);
							return true;
						}
					}
				}
				break;
			}
		}
		return false;
	}
private:
	EventEmitterPtr mEmitter;
	std::shared_ptr<KeyboardInputDevice> mKeyboard;
	std::shared_ptr<MouseInputDevice> mMouse;
};

InputPlatformQt::InputPlatformQt() :
	mEmitter(std::make_shared<EventEmitter>())
{
	mKeyboard = std::make_shared<KeyboardInputDevice>();
	mMouse = std::make_shared<MouseInputDevice>();

	mEventFilter = std::make_unique<InputPlatformQtEventFilter>(mEmitter, mKeyboard, mMouse);
	QApplication::instance()->installEventFilter(mEventFilter.get());
}

InputPlatformQt::~InputPlatformQt()
{
	QApplication::instance()->removeEventFilter(mEventFilter.get());
}

std::vector<InputDevicePtr> InputPlatformQt::getInputDevicesOfType(InputDeviceType type) const
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

void InputPlatformQt::setEnabled(bool enabled)
{
	mKeyboard->setEnabled(enabled);
	mMouse->setEnabled(enabled);
}

} // namespace skybolt