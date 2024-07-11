/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltSim/Spatial/LatLonAlt.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("LatLonAlt elements accessible by [] operator")
{
	sim::LatLonAlt v(0.1, 0.2, 0.3);
	CHECK(v[0] == 0.1);
	CHECK(v[1] == 0.2);
	CHECK(v[2] == 0.3);
}
