/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InputSystem.h"
#include "InputPlatform.h"
#include "LogicalAxis.h"
#include <SkyboltCommon/VectorUtility.h>

namespace skybolt {

InputSystem::InputSystem(const InputPlatformPtr& inputPlatform, const std::vector<LogicalAxisPtr>& axes) :
	mInputPlatform(inputPlatform),
	mAxes(axes)
{
	assert(mInputPlatform);
}

void InputSystem::advanceWallTime(sim::SecondsD newTime, sim::SecondsD dt)
{
	mDtWallClock += dt;
}

void InputSystem::updateState()
{
	mInputPlatform->update();

	for (const LogicalAxisPtr& axis : mAxes)
	{
		axis->update(mDtWallClock);
	}
	mDtWallClock = 0;
}

} // namespace skybolt