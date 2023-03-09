/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Position.h"
#include "Geocentric.h"
#include "GreatCircle.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <assert.h>

namespace skybolt {
namespace sim {

GeocentricPosition toGeocentric(const Position& position)
{
	switch (position.type)
	{
		case Position::TypeGeocentric:
		{
			return static_cast<const GeocentricPosition&>(position);
		}
		case Position::TypeLatLonAlt:
		{
			const LatLonAltPosition& lla = static_cast<const LatLonAltPosition&>(position);
			return GeocentricPosition(llaToGeocentric(lla.position, earthRadius()));
		}
		case Position::TypeNed:
		{
			//TODO: implement
			assert(!"Not implemented");
		}
		default:
			assert(!"Not implemented");
	}
	return GeocentricPosition(math::dvec3Zero());
}

LatLonAltPosition toLatLonAlt(const Position& position)
{
	switch (position.type)
	{
	case Position::TypeGeocentric:
	{
		const GeocentricPosition& geocentricPos = static_cast<const GeocentricPosition&>(position);
		return LatLonAltPosition(geocentricToLla(geocentricPos.position, earthRadius()));
	}
	case Position::TypeLatLonAlt:
	{
		return static_cast<const LatLonAltPosition&>(position);
	}
	case Position::TypeNed:
	{
		// TODO: implement
		assert(!"Not implemented");
	}
	default:
		assert(!"Not implemented");
	}
	return LatLonAltPosition(LatLonAlt{});
}

} // namespace skybolt
} // namespace sim