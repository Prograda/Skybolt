/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Physics/Atmosphere.h>
#include <SkyboltCommon/NumericComparison.h>
#include <catch2/catch.hpp>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("Atmosphere density matches reference data")
{
    Atmosphere atm(288.15, 101300, 0.0065, 9.8, 0.0289644);
    // From 1976 Standard Atmosphere
	CHECK(almostEqualFracEpsilon(0.909122, atm.getDensity(3000), 0.001));
	CHECK(almostEqualFracEpsilon(0.525168, atm.getDensity(8000), 0.001));
}
