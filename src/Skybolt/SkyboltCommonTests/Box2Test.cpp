/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/Box2.h>

using namespace skybolt;
using namespace glm;

TEST_CASE("Box vs box intersection")
{
	Box2 a(vec2(-1, -2), vec2(3, 4));
	Box2 b(vec2(3.5, 1), vec2(30, 40));

	CHECK(!a.intersects(b));

	b = Box2(vec2(2.9f, 1), vec2(30, 40));
	CHECK(a.intersects(b));

	b = Box2(vec2(-1, -2), vec2(0, 2));
	CHECK(a.intersects(b));

	b = Box2(vec2(-10, -20), vec2(-1, -1.8f));
	CHECK(a.intersects(b));
}
