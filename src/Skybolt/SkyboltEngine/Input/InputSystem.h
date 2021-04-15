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
	InputSystem(const InputPlatformPtr& inputPlatform, vis::Window* window, const std::vector<LogicalAxisPtr>& axes);

	void updatePostDynamics(const System::StepArgs& args);

private:
	InputPlatformPtr mInputPlatform;
	vis::Window* mWindow;
	std::vector<LogicalAxisPtr> mAxes;
};

} // namespace skybolt