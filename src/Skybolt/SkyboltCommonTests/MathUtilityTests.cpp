/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt::math;

TEST_CASE("Convert euler to quaternion")
{
	glm::vec3 euler(0.1, 0.2, 0.3);
	glm::quat ori = quatFromEuler(euler);

	glm::vec3 finalEuler = eulerFromQuat(ori);
	CHECK(glm::distance(euler, finalEuler) < 1e-6f);
}

TEST_CASE("getOrthonormalBasis")
{
	glm::vec3 tangent, bitangent;
	getOrthonormalBasis(glm::vec3(1, 0, 0), tangent, bitangent);

	CHECK(glm::distance(tangent, glm::vec3(0, 1, 0)) < 1e-6f);
	CHECK(glm::distance(bitangent, glm::vec3(0, 0, 1)) < 1e-6f);
}