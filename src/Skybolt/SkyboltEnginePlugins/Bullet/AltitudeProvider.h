/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Spatial/LatLon.h>

namespace skybolt {
namespace sim {

class AltitudeProvider
{
public:
	virtual ~AltitudeProvider() {}

	//! @return altitude above sea level. Positive is up.
	virtual double get(const LatLon& position) const = 0;
};

} // namespace sim
} // namespace skybolt
