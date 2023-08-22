/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "LatLon.h"
#include <assert.h>
#include <tuple>

namespace skybolt {
namespace sim {

struct LatLonAlt
{
	LatLonAlt() {}
	LatLonAlt(double lat, double lon, double alt) : lat(lat), lon(lon), alt(alt) {}

	bool operator == (const LatLonAlt& other) const
	{
		return std::tie(lat, lon, alt) == std::tie(other.lat, other.lon, other.alt);
	}

	double operator[] (int i) const
	{
		switch (i)
		{
			case 0: return lat;
			case 1: return lon;
			case 2: return alt;
		}
		assert(!"Invalid index");
		return alt;
	}

	double& operator[] (int i)
	{
		switch (i)
		{
			case 0: return lat;
			case 1: return lon;
			case 2: return alt;
		}
		assert(!"Invalid index");
		return alt;
	}

	double lat; //!< radians
	double lon; //!< radians
	double alt; //!< meters above sea level
};

inline sim::LatLon toLatLon(const sim::LatLonAlt& lla)
{
	return sim::LatLon(lla.lat, lla.lon);
}

inline sim::LatLonAlt toLatLonAlt(const sim::LatLon& ll, double alt)
{
	return sim::LatLonAlt(ll.lat, ll.lon, alt);
}

} // namespace skybolt
} // namespace sim