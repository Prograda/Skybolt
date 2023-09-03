/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "UpdateLoopUtility.h"
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/VisRoot.h>

namespace skybolt {

using namespace sim;

void runMainLoop(vis::VisRoot& visRoot, EngineRoot& engineRoot, UpdateLoop::ShouldExit shouldExit, SimPausedPredicate paused)
{
	// Run main loop
	auto systemRegistry = engineRoot.systemRegistry;
	auto simStepper = std::make_shared<SimStepper>(systemRegistry);

	SecondsD minFrameDuration = 0.01;
	SecondsD currentWallTime = 0;

	UpdateLoop loop(minFrameDuration);
	loop.exec([&](float dtWallClock) {
		SecondsD dtSim = paused() ? 0.0 : dtWallClock;
		simStepper->step(dtSim);

		for (const auto& system : *systemRegistry)
		{
			system->advanceWallTime(currentWallTime, dtWallClock);
		}
		currentWallTime += dtWallClock;

		return visRoot.render();
	}, shouldExit);
}

} // namespace skybolt