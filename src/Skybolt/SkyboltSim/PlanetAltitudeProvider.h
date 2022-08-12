/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Spatial/LatLon.h>
#include <optional>

namespace skybolt {
namespace sim {

class PlanetAltitudeProvider
{
public:
	//! Get altitude above sea level, positive is up.
	virtual double getAltitude(const sim::LatLon& position) const = 0;
};

class AsyncPlanetAltitudeProvider
{
public:
	//! Get altitude above sea level, positive is up.
	//! If tile is not immediately available, requests to load tile on a background thread
	//! and immediately returns empty optional.
	virtual std::optional<double> getAltitudeOrRequestLoad(const sim::LatLon& position) const = 0;
};

} // namespace sim
} // namespace skybolt