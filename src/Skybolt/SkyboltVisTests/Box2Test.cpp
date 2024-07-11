/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltVis/OsgBox2.h>

using namespace skybolt::vis;
using namespace osg;

TEST_CASE("Box vs box intersection")
{
	Box2f a(Vec2f(-1, -2), Vec2f(3, 4));
	Box2f b(Vec2f(3.5, 1), Vec2f(30, 40));

	CHECK(!a.intersects(b));

	b = Box2f(Vec2f(2.9f, 1), Vec2f(30, 40));
	CHECK(a.intersects(b));

	b = Box2f(Vec2f(-1, -2), Vec2f(0, 2));
	CHECK(a.intersects(b));

	b = Box2f(Vec2f(-10, -20), Vec2f(-1, -1.8f));
	CHECK(a.intersects(b));
}
