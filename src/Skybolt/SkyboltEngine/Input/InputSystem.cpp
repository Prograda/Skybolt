/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InputSystem.h"
#include "InputPlatform.h"
#include "LogicalAxis.h"
#include <SkyboltVis/Window/Window.h>

namespace skybolt {

InputSystem::InputSystem(const InputPlatformPtr& inputPlatform, vis::Window* window, const std::vector<LogicalAxisPtr>& axes) :
	mInputPlatform(inputPlatform),
	mWindow(window),
	mAxes(axes)
{
	assert(mInputPlatform);
	assert(mWindow);
}

void InputSystem::updatePostDynamics(const System::StepArgs& args)
{
	mInputPlatform->update();
	mInputPlatform->setWindowWidth(mWindow->getWidth());
	mInputPlatform->setWindowHeight(mWindow->getHeight());

	for (const LogicalAxisPtr& axis : mAxes)
	{
		axis->update(args.dtWallClock);
	}
}

} // namespace skybolt