/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "InputPlatform.h"
#include <SkyboltCommon/Event.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <osgGA/GUIEventHandler>
#include <optional>
#include <string>

namespace osgViewer {
	class View;
}

namespace skybolt {

class MouseInputDeviceOsg : public InputDevice
{
public:
	MouseInputDeviceOsg(const EventEmitterPtr& emitter);

	~MouseInputDeviceOsg();

	void setEnabled(bool enabled) override;

	size_t getButtonCount() const override { return 7; }

	bool isButtonPressed(size_t i) const override
	{
		return mEnabled && mPressedButtons.find(i) != mPressedButtons.end();
	}

	size_t getAxisCount() const override { return 0; }
	float getAxisState(size_t i) const override { assert(0); return 0.0f; }

	const std::string& getName() const override
	{
		static std::string mouse = "Mouse";
		return mouse;
	}

	bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

	void enablePointerWrap(bool wrap);

private:
	void setMouseState(MouseEvent& event, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

private:
	EventEmitterPtr mEmitter;
	std::set<int> mPressedButtons;
	bool mEnabled = true;
	std::optional<glm::vec2> mPrevPosition;
	bool mWrapEnabled = true;
};

class InputPlatformOsg : public InputPlatform
{
public:
	InputPlatformOsg(const osg::ref_ptr<osgViewer::View>& view);
	~InputPlatformOsg() override;

	void setEnabled(bool enabled) override;

	std::vector<InputDevicePtr> getInputDevicesOfType(InputDeviceType type) const override;

	virtual EventEmitterPtr getEventEmitter() const override { return mEmitter; }

private:
	void addEventHandler(const osg::ref_ptr<osgGA::GUIEventHandler>& handler);

private:
	osg::ref_ptr<osgViewer::View> mView;
	EventEmitterPtr mEmitter;
	std::shared_ptr<class KeyboardInputDeviceOsg> mKeyboard;
	std::shared_ptr<MouseInputDeviceOsg> mMouse;
	std::vector<osg::ref_ptr<osgGA::GUIEventHandler>> mEventHandlers;
};

} // namespace skybolt