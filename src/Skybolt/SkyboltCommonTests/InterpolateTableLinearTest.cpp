/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltCommon/Math/InterpolateTableLinear.h>
#include <catch2/catch.hpp>

using namespace skybolt;
using namespace math;

TEST_CASE("findInterpolationPoint")
{
	std::vector<double> xData = { 4, 5, 7 };

	SECTION("Null returned when interpolating empty vector")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint({}, 2, /* extrapolate */ true);
		CHECK(!point.is_initialized());
	}

	SECTION("Extrapolate below lower bound")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint(xData, 2, /* extrapolate */ true);
		REQUIRE(point.is_initialized());
		CHECK(point->bounds.first == 0);
		CHECK(point->bounds.last == 1);
		CHECK(point->weight == -2);
	}

	SECTION("Extrapolate above upper bound")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint(xData, 8, /* extrapolate */ true);
		REQUIRE(point.is_initialized());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 1.5);
	}

	SECTION("Clamp below lower bound")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint(xData, 2, /* extrapolate */ false);
		REQUIRE(point.is_initialized());
		CHECK(point->bounds.first == 0);
		CHECK(point->bounds.last == 1);
		CHECK(point->weight == 0);
	}

	SECTION("Clamp above upper bound")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint(xData, 8, /* extrapolate */ false);
		REQUIRE(point.is_initialized());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 1);
	}

	SECTION("Interpolate")
	{
		boost::optional<InterpolationPoint> point = findInterpolationPoint(xData, 5.5, /* extrapolate */ false);
		REQUIRE(point.is_initialized());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 0.25);
	}
}
