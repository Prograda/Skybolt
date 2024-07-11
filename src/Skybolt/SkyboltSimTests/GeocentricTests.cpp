/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/NumericComparison.h>

using namespace skybolt;
using namespace skybolt::sim;

constexpr double epsilon = 1e-8f;
constexpr double altitude = 123;

TEST_CASE("Zero lat, zero long converts to +X")
{
	LatLonAlt lla(0, 0, altitude);

	Vector3 pos = llaToGeocentric(lla, earthRadius());
	CHECK(almostEqual(Vector3(earthRadius() + altitude, 0, 0), pos, epsilon));
}

TEST_CASE("zero lat, half pi long converts to +Y")
{
	LatLonAlt lla(0, math::halfPiD(), altitude);

	Vector3 pos = llaToGeocentric(lla, earthRadius());
	CHECK(almostEqual(Vector3(0, earthRadius() + altitude, 0), pos, epsilon));
}

TEST_CASE("half pi lat converts to +Z")
{
	LatLonAlt lla(math::halfPiD(), 0, altitude);

	Vector3 pos = llaToGeocentric(lla, earthRadius());
	CHECK(almostEqual(Vector3(0, 0, earthRadius() + altitude), pos, epsilon));
}

TEST_CASE("Geocentric to LLA is reciprocal of LLA to Geocentric")
{
	LatLonAlt lla(0.1, 0.2, altitude);

	Vector3 pos = llaToGeocentric(lla, earthRadius());
	LatLonAlt lla2 = geocentricToLla(pos, earthRadius());

	CHECK(almostEqual(lla, lla2, epsilon));
}

TEST_CASE("LLA to Geocentric LTP Orientation")
{
	LatLonAlt lla(0.1, 0.2, altitude);

	Vector3 pos = llaToGeocentric(lla, earthRadius());
	Quaternion ori = latLonToGeocentricLtpOrientation(toLatLon(lla));
	Vector3 pos2 = ori * Vector3(0, 0, -earthRadius() - altitude);

	CHECK(almostEqual(pos, pos2, epsilon));
}
