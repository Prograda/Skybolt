/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "InputPlatform.h"
#include "SkyboltCommon/Event.h"

#include <osgGA/GUIEventHandler>
#include <string>

namespace osgViewer
{
	class Viewer;
}

namespace skybolt {

class InputPlatformOsg : public InputPlatform
{
public:
	InputPlatformOsg(const std::weak_ptr<osgViewer::Viewer>& viewer);
	~InputPlatformOsg() override;

	void setEnabled(bool enabled) override;

	std::vector<InputDevicePtr> getInputDevicesOfType(InputDeviceType type) const override;

	virtual EventEmitterPtr getEventEmitter() const override { return mEmitter; }

private:
	void addEventHandler(const osg::ref_ptr<osgGA::GUIEventHandler>& handler);

private:
	std::weak_ptr<osgViewer::Viewer> mViewer;
	EventEmitterPtr mEmitter;
	std::shared_ptr<class KeyboardInputDevice> mKeyboard;
	std::shared_ptr<class MouseInputDevice> mMouse;
	std::vector<osg::ref_ptr<osgGA::GUIEventHandler>> mEventHandlers;
};

} // namespace skybolt