/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <assert.h>

namespace skybolt {
namespace sim {

struct LatLon
{
	LatLon() {}
	LatLon(double lat, double lon) : lat(lat), lon(lon) {}

	bool operator == (const LatLon& other) const
	{
		return (lat == other.lat && lon == other.lon);
	}

	double operator[] (int i) const
	{
		assert(i == 0 || i == 1);
		return i ? lon : lat;
	}

	double& operator[] (int i)
	{
		assert(i == 0 || i == 1);
		return i ? lon : lat;
	}

	double lat; //!< radians
	double lon; //!< radians
};

} // namespace skybolt
} // namespace sim