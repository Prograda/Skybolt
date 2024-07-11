/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Geocentric.h"
#include "SimMath.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

sim::Vector3 llaToGeocentric(const sim::LatLonAlt& lla, double planetRadius)
{
	double cosLat = cos(lla.lat);
	return sim::Vector3(cos(lla.lon) * cosLat, sin(lla.lon) * cosLat, sin(lla.lat)) * (planetRadius + lla.alt);
}

sim::LatLonAlt geocentricToLla(const sim::Vector3& pos, double planetRadius)
{
	sim::LatLonAlt lla;
	double denom = pos.x*pos.x + pos.y*pos.y;
	lla.lat = denom > 0.0 ? atan(pos.z / sqrt(denom)) : 0.0;
	lla.lon = atan2(pos.y, pos.x);
	lla.alt = glm::length(pos) - planetRadius;
	return lla;
}

sim::LatLon geocentricToLatLon(const sim::Vector3& pos)
{
	sim::LatLon ll;
	double denom = pos.x*pos.x + pos.y*pos.y;
	ll.lat = denom > 0.0 ? atan(pos.z / sqrt(denom)) : 0.0;
	ll.lon = atan2(pos.y, pos.x);
	return ll;
}

sim::Quaternion latLonToGeocentricLtpOrientation(const sim::LatLon& latLon)
{
	return glm::angleAxis(latLon.lon, sim::Vector3(0, 0, 1)) * glm::angleAxis(latLon.lat + skybolt::math::halfPiD(), sim::Vector3(0, -1, 0)); // Note: sim::Quaternion rotation order is different to OSG::Quat
}

sim::Matrix3 geocentricToLtpOrientation(const sim::Vector3& pos)
{
	sim::Vector3 down = -glm::normalize(pos);
	sim::Vector3 north(0, 0, -1);

	sim::Vector3 east = glm::normalize(glm::cross(down, north));
	north = glm::cross(east, down);

	sim::Matrix3 orientation(north, east, down);
	return orientation;
}

} // namespace skybolt
} // namespace sim