/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;
using namespace skybolt::sim;

static constexpr double epsilon = 1e-14;

TEST_CASE("Lat Lon to North East")
{
	LatLon origin(0.1, 0.2);
	LatLon latLon(0.3, 0.4);

	glm::dvec2 ne = latLonToCartesianNe(origin, latLon);
	LatLon latLon2 = cartesianNeToLatLon(origin, ne);

	CHECK(abs(latLon.lat - latLon2.lat) < epsilon);
	CHECK(abs(latLon.lon - latLon2.lon) < epsilon);
}

TEST_CASE("Calc bearing from/to LatLon points")
{
	CHECK(calcBearing(sim::LatLon(0.1, 0.2), sim::LatLon(0.2, 0.2)) == Approx(0.0).margin(epsilon));
	CHECK(calcBearing(sim::LatLon(0.0, 0.2), sim::LatLon(0.0, 0.3)) == Approx(math::halfPiD()).margin(epsilon));
}

TEST_CASE("Move distance and bearing from a point")
{
	auto result = moveDistanceAndBearing(sim::LatLon(0.1, 0.2), 1000000, 0.123);
	CHECK(calcDistance(sim::LatLon(0.1, 0.2), result) == Approx(1000000).margin(epsilon));
	CHECK(calcBearing(sim::LatLon(0.1, 0.2), result) == Approx(0.123).margin(epsilon));
}
