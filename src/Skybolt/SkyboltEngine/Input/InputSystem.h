/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltSim/System/System.h"
#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Vec2f>
#include <vector>

namespace skybolt {

class InputSystem : public sim::System
{
public:
	InputSystem(const InputPlatformPtr& inputPlatform, const std::vector<LogicalAxisPtr>& axes = {});

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::Input, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void advanceWallTime(sim::SecondsD newTime, sim::SecondsD dt) override;
	void updateState();

private:
	InputPlatformPtr mInputPlatform;
	std::vector<LogicalAxisPtr> mAxes;
	sim::SecondsD mDtWallClock = 0;
};

} // namespace skybolt