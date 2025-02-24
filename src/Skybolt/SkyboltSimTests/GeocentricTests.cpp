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

TEST_CASE("Calculate LTP orientation at a geocentric point")
{
	LatLonAlt lla(0.1, 0.2, altitude);
	Vector3 pos = llaToGeocentric(lla, earthRadius());
	Quaternion ori = geocentricToLtpOrientation(pos);

	SECTION("LTP +X axis is north")
	{
		// Create position to the north
		LatLonAlt northLla = lla;
		northLla.lat += 0.0001;
		Vector3 northPos = llaToGeocentric(northLla, earthRadius());

		// Get north vector in geocentric coordinates
		Vector3 northDir = glm::normalize(northPos - pos);

		// Convert position to NED axes
		Vector3 posNed = northDir * ori;
		CHECK(glm::dot(posNed, Vector3(1, 0, 0)) == Approx(1.0).margin(epsilon));
	}

	SECTION("LTP +Z axis is down")
	{
		Vector3 pos2 = ori * Vector3(0, 0, -earthRadius() - altitude);
		CHECK(almostEqual(pos, pos2, epsilon));
	}
}

TEST_CASE("Calculate LTP orientation at a LatLonAlt")
{
	// Calculate geocentric LTP orientation at a LLA point
	LatLonAlt lla(0.1, 0.2, altitude);
	Vector3 pos = llaToGeocentric(lla, earthRadius());
	Quaternion ori = latLonToGeocentricLtpOrientation(toLatLon(lla));
	Quaternion oriExpected = geocentricToLtpOrientation(pos);

	// Check that orientation equals result from geocentricToLtpOrientation()
	CHECK(almostEqual(ori * Vector3(1,2,3), oriExpected * Vector3(1,2,3), epsilon));
}
