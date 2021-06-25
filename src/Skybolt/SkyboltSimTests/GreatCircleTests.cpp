/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltSim/Spatial/GreatCircle.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("Lat Lon to North East")
{
	LatLon origin(0.1, 0.2);
	LatLon latLon(0.3, 0.4);

	glm::dvec2 ne = latLonToCartesianNe(origin, latLon);
	LatLon latLon2 = cartesianNeToLatLon(origin, ne);

	CHECK(abs(latLon.lat - latLon2.lat) < 1e-8f);
	CHECK(abs(latLon.lon - latLon2.lon) < 1e-8f);
}
