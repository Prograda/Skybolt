/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GreatCircle.h"
#include <math.h>

namespace skybolt {
namespace sim {

using sim::LatLon;

template <typename T>
inline T sqr(T v) {return v*v;}

double calcDistance(const LatLon& p1, const LatLon& p2)
{
	double dlon = p2.lon - p1.lon;
	double dlat = p2.lat - p1.lat;
	double a = sqr(sin(dlat/2)) + cos(p1.lat) * cos(p2.lat) * sqr(sin(dlon/2));
	double c = 2 * atan2( sqrt(a), sqrt(1-a) );
	return earthRadius() * c;
}

double calcBearing(const LatLon& p1, const LatLon& p2)
{
	double y = sin(p2.lon-p1.lon) * cos(p2.lat);
	double x = cos(p1.lat)*sin(p2.lat) - sin(p1.lat)*cos(p2.lat)*cos(p2.lat-p1.lat);
	return atan2(y, x);
}

glm::dvec2 latLonToCartesianNe(const LatLon& origin, const LatLon& position)
{
	double dlat = position.lat - origin.lat;
	double dlon = position.lon - origin.lon;

	return glm::dvec2(dlat * earthRadius(), dlon * earthRadius() * cos(position.lat));
}

LatLon cartesianNeToLatLon(const LatLon& origin, const glm::dvec2& position)
{
	double oneOnEarthRadius = 1.0 / earthRadius();
	double dlat = double(position.x) * oneOnEarthRadius;

	LatLon result;
	result.lat = origin.lat + dlat;

	double dlon = double(position.y) * oneOnEarthRadius / cos(result.lat);
	result.lon = origin.lon + dlon;
	return result;
}

sim::LatLon moveDistanceAndBearing(const sim::LatLon& origin, double distance, double bearing)
{
	sim::LatLon result;

	const double distanceRadians = distance / earthRadius();
	const double sinDist = sin(distanceRadians);
	const double cosDist = cos(distanceRadians);
	const double cosLat = cos(origin.lat);
	const double sinLat = sin(origin.lat);

	result.lat = asin(sinLat * cosDist + cosLat * sinDist * cos(bearing));
	result.lon = origin.lon + atan2(sin(bearing) * sinDist * cosLat,
		cosDist - sinLat * sin(result.lat));

	return result;
}

} // namespace skybolt
} // namespace sim