/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "UpdateLoopUtility.h"
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/Window/Window.h>

namespace skybolt {

using namespace sim;

void runMainLoop(vis::Window& window, EngineRoot& engineRoot, UpdateLoop::ShouldExit shouldExit, SimPausedPredicate paused)
{
	// Run main loop
	auto simStepper = std::make_shared<SimStepper>(engineRoot.systemRegistry);

	double prevElapsedTime = 0;
	double minFrameDuration = 0.01;

	UpdateLoop loop(minFrameDuration);
	loop.exec([&](float dtWallClock) {
		System::StepArgs args;
		args.dtSim = paused() ? 0.0 : dtWallClock;
		args.dtWallClock = dtWallClock;
		simStepper->step(args);
		return window.render();
	}, shouldExit);
}

} // namespace skybolt