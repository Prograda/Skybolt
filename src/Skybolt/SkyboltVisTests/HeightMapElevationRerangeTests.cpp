/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Elevation/ElevationRerange.h>

using namespace skybolt;
using namespace skybolt::vis;


TEST_CASE("Test create rerange for UInt16 with elevation bounds")
{
	ElevationRerange r = rerangeElevationFromUInt16WithElevationBounds(1000, 5000);
	CHECK(getColorValueForElevation(r, 1000) == 0);
	CHECK(getColorValueForElevation(r, 5000) == 65535);
}

TEST_CASE("Test convert between color value and elevation")
{
	ElevationRerange r = { 2, 3 };

	int colorValue = 123;
	float elevation = getElevationForColorValue(r, colorValue);
	CHECK(colorValue == getColorValueForElevation(r, elevation));
}
