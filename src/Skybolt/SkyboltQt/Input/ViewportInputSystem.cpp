/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportInputSystem.h"
#include <SkyboltEngine/Input/InputPlatform.h>

using namespace skybolt;

ViewportInputSystem::ViewportInputSystem(const skybolt::InputPlatformPtr& inputPlatform, CameraInputAxes axes) :
	CameraInputSystem(inputPlatform, std::move(axes))
{
	setMouseEnabled(false);
	setKeyboardEnabled(false);
}

void ViewportInputSystem::onEvent(const Event& event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		if (mouseEvent->type == MouseEvent::Type::Released)
		{
			mInputPlatform->setEnabled(false);
		}
	}
	CameraInputSystem::onEvent(event);
}