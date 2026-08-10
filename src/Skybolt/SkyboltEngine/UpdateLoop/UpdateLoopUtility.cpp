/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "UpdateLoopUtility.h"
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>

namespace skybolt {

using namespace sim;

void runMainLoop(EngineRoot& engineRoot, UpdateLoop::Updatable updatable, UpdateLoop::ShouldExit shouldExit, SimPausedPredicate paused)
{
	// Run main loop
	auto systemRegistry = engineRoot.systemRegistry;
	sim::TimeSource& timeSource = *engineRoot.scenario->timeSource;

	SecondsD minFrameDuration = 0.01;
	SecondsD currentWallTime = 0;

	UpdateLoop loop(minFrameDuration);
	loop.exec([&](float dtWallClock) {
		SecondsD dtSim = paused() ? 0.0 : dtWallClock;
		timeSource.advanceTime(dtSim);

		for (const auto& system : *systemRegistry)
		{
			system->advanceWallTime(currentWallTime, dtWallClock);
		}
		currentWallTime += dtWallClock;

		return updatable(dtWallClock);
	}, shouldExit);
}

} // namespace skybolt