/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/TimeSource.h>

namespace skybolt {

struct Scenario
{
	double startJulianDate = 2457982.9;
	TimeSource timeSource = TimeRange(0, 120);
};

} // namespace skybolt