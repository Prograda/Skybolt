/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Spatial/LatLon.h>
#include <optional>
#include <tuple>

namespace skybolt {
namespace sim {

class PlanetAltitudeProvider
{
public:
	struct AltitudeResult
	{
		double altitude; //!< Altitude above sea level, positive is up.

		//! True if altitude value is provisional, meaning that a more accurate value may be provided in the future.
		//! This is intended to accomodate asynchronous elevation data sources which can return a provisional estimated value
		//! until the most accurate data is available.
		bool provisional;

		static AltitudeResult provisionalValue(double altitude)
		{
			AltitudeResult r;
			r.altitude = altitude;
			r.provisional = true;
			return r;
		}

		static AltitudeResult finalValue(double altitude)
		{
			AltitudeResult r;
			r.altitude = altitude;
			r.provisional = false;
			return r;
		}

		bool operator == (const AltitudeResult& other) const {
			return std::tie(altitude, provisional) == std::tie(other.altitude, other.provisional);
		};
	};

	virtual AltitudeResult getAltitude(const sim::LatLon& position) const = 0;
};

} // namespace sim
} // namespace skybolt