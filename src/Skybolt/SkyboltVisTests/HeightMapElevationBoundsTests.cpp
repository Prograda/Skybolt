/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h>

#include <osg/Image>

using namespace skybolt;
using namespace skybolt::vis;

TEST_CASE("Test get and set HeightMapElevationBounds image data")
{
	osg::ref_ptr<osg::Image> image = new osg::Image();

	HeightMapElevationBounds bounds(10, 20);

	setHeightMapElevationBounds(*image, bounds);

	std::optional<HeightMapElevationBounds> bounds2 = getHeightMapElevationBounds(*image);
	REQUIRE(bounds2);
	CHECK(bounds == *bounds2);
}
