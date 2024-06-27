/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltQt/Viewport/ScreenTransformUtil.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

static constexpr double epsilon = 1e-8f;

TEST_CASE("Convert world to screen coordinates")
{
	sim::CameraState cameraState;
	cameraState.nearClipDistance = 0.1;
	cameraState.farClipDistance = 1000;
	cameraState.fovY = 0.5;

	double aspectRatio = 1.2;

	glm::dmat4 viewProjTransform = makeViewProjTransform(skybolt::sim::Vector3(1,2,3), skybolt::math::dquatIdentity(), cameraState, aspectRatio);
	std::optional<sim::Vector3> r = worldToScreenNdcPoint(viewProjTransform, sim::Vector3(10, 2, 3));
	REQUIRE(r);
	CHECK(r->x == Approx(0.5).margin(epsilon));
	CHECK(r->y == Approx(0.5).margin(epsilon));

	skybolt::sim::Vector3 worldPoint = screenToWorldPoint(glm::inverse(viewProjTransform), *r);
	CHECK(worldPoint.x == Approx(10).margin(epsilon));
	CHECK(worldPoint.y == Approx(2).margin(epsilon));
	CHECK(worldPoint.z == Approx(3).margin(epsilon));
}
