/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/ElevationProvider/HeightMapElevationProvider.h>
#include <SkyboltCommon/NumericComparison.h>

using namespace skybolt;
using namespace skybolt::vis;

static float heightFunction(float x, float y)
{
	return x * 2 + y;
}

TEST_CASE("Test HeightMapElevationProvider returns correct elevations")
{
	HeightMapElevationRerange rerange = rerangeElevationFromUInt16WithElevationBounds(-100, 100);

	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(4, 4, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	
	uint16_t* p = reinterpret_cast<uint16_t*>(image->data());
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			float elevation = heightFunction(x, y);
			p[x + y * 4] = getColorValueForElevation(rerange, elevation);
		}
	}

	Box2f bounds(osg::Vec2f(1, 2), osg::Vec2f(1+2, 2+4));

	HeightMapElevationProvider provider(image, rerange, bounds);

	const float epsilon = 0.01f;

	// Check height at opposite corners
	CHECK(provider.get(1, 2) == Approx(0).margin(epsilon));
	CHECK(provider.get(3, 6) == Approx(9).margin(epsilon));

	// Check height in center of image
	CHECK(provider.get(2, 4) == Approx(heightFunction(2, 2)).margin(epsilon));

	// Check height in center of first texel
	CHECK(provider.get(1.25, 2.5) == Approx(heightFunction(0.5, 0.5)).margin(epsilon));

	// Check that out of bounds coordinates are clamped to nearest edge
	CHECK(provider.get(-1, -1) == Approx(0).margin(epsilon));
	CHECK(provider.get(10, 10) == Approx(9).margin(epsilon));
}
