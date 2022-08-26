/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltSim/Spatial/Frustum.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;
using namespace skybolt::sim;

constexpr double epsilon = 1e-8;

TEST_CASE("transformToScreenSpace transforms points")
{
	Frustum f;
	f.origin = Vector3(1,2,3);
	f.orientation = glm::angleAxis(skybolt::math::halfPiD(), Vector3(0,0,1));
	f.fieldOfViewHorizontal = skybolt::math::halfPiD();
	f.fieldOfViewVertical = skybolt::math::halfPiD();

	// Point at origin
	{
		Vector3 pos = transformToScreenSpace(f, f.origin);
		CHECK(pos.x == Approx(0.0).margin(epsilon));
		CHECK(pos.y == Approx(0.0).margin(epsilon));
		CHECK(pos.z == Approx(0.0).margin(epsilon));
	}

	// Point at screen corner
	{
		Vector3 pos = transformToScreenSpace(f, f.origin + Vector3(-10, 10, -10));
		CHECK(pos.x == Approx(1.0).margin(epsilon));
		CHECK(pos.y == Approx(1.0).margin(epsilon));
		CHECK(pos.z == Approx(10.0).margin(epsilon));
	}
}
