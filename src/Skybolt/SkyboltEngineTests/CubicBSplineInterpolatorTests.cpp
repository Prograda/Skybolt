/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltEngine/Sequence/Interpolator/CubicBSplineInterpolator.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Vec3d>

using namespace skybolt;

constexpr float epsilon = 1e-8f;

TEST_CASE("CubicBSplineInterpolator has expected values")
{
	// Tested against https://www.desmos.com/calculator/d1ofwre0fr
	std::vector<double> t = { 0, 1.0, 2.0 };
	std::vector<double> y = { 0.5, 1.0, 0.5 };

	CubicBSplineInterpolatorD interpolator(
		[&] { return t.size(); },
		[&](int i) { return y[i]; },
		[&](int i) { return t[i]; });

	double epsilon = 1e-7;
	CHECK(interpolator.interpolate(0, 1, 1.0) == Approx(1.0).margin(epsilon));
	CHECK(interpolator.interpolate(0, 1, 0.5) == Approx(0.75).margin(epsilon));
}

TEST_CASE("CubicBSplineInterpolator has expected tangent at middle vertex")
{
	// Tested against https://www.desmos.com/calculator/d1ofwre0fr
	std::vector<double> t = { 1.0, 1.5, 5.5 };
	std::vector<double> y = { 2.0, 2.0, 6.0 };

	CubicBSplineInterpolatorD interpolator(
		[&] { return t.size(); },
		[&](int i) { return y[i]; },
		[&](int i) { return t[i]; });

	double epsilon = 1e-7;

	SECTION("Tangent slightly to left of point")
	{
		double dt = 0.5 * epsilon;
		double measuredGradient = (interpolator.interpolate(0, 1, 1.0) - interpolator.interpolate(0, 1, 1.0 - epsilon)) / dt;
		CHECK(glm::tan(22.5 * math::degToRadD()) == Approx(measuredGradient).margin(1e-6));
	}

	SECTION("Tangent slightly to right of point")
	{
		double dt = 4.0 * epsilon;
		double measuredGradient = (interpolator.interpolate(1, 2, epsilon) - interpolator.interpolate(1, 2, 0.0)) / dt;
		CHECK(glm::tan(22.5 * math::degToRadD()) == Approx(measuredGradient).margin(1e-6));
	}
}
