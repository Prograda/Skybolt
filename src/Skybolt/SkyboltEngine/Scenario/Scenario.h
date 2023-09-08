/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/ObservableValue.h>
#include <SkyboltEngine/TimeSource.h>
#include <SkyboltSim/World.h>

namespace skybolt {

enum class TimelineMode
{
	Live, //!< Simulation can only advance forward in time in contiguious time-steps. For example, dynamic model simulation.
	Free //!< User can jump to any point in time as desired. For example, trajectory data playback.
};

struct Scenario
{
	double startJulianDate = 2457982.9;
	TimeSource timeSource = TimeRange(0, 6000);
	ObservableValue<TimelineMode> timelineMode = TimelineMode::Live;

	sim::World world;
};

double getCurrentJulianDate(const Scenario& scenario);

} // namespace skybolt