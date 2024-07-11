/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltCommon/Event.h"
#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltQt/Viewport/ViewportMouseEventHandler.h>

namespace skybolt {

class InputPlatformQt : public InputPlatform
{
	friend class InputPlatformQtEventFilter;
public:
	//!@param windowHandle is the handle for the window. On windows this is the HWND number as a string.
	InputPlatformQt();
	~InputPlatformQt() override;

public: // InputPlatform interface
	void setEnabled(bool enabled) override;

	std::vector<InputDevicePtr> getInputDevicesOfType(InputDeviceType type) const override;

	virtual EventEmitterPtr getEventEmitter() const override { return mEmitter; }

private:
	EventEmitterPtr mEmitter;
	std::shared_ptr<struct KeyboardInputDevice> mKeyboard;
	std::shared_ptr<struct MouseInputDevice> mMouse;
	std::unique_ptr<class InputPlatformQtEventFilter> mEventFilter;
};

} // namespace skybolt