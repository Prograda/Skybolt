/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt::math;

TEST_CASE("Component-wise multiply scalar by vector")
{
	float s = 5;
	glm::vec2 v(2, 4);
	glm::vec2 r = componentWiseMultiply(s, v);
	CHECK(glm::distance(v * s, r) < 1e-8f);
}

TEST_CASE("Component-wise multiply vector by vector")
{
	glm::vec2 a(2, 4);
	glm::vec2 b(3, 5);
	glm::vec2 r = componentWiseMultiply(a, b);
	CHECK(glm::distance(a * b, r) < 1e-8f);
}

TEST_CASE("Component-wise divide vector by vector")
{
	glm::vec2 a(10, 5);
	glm::vec2 b(5, 2);
	glm::vec2 r = componentWiseDivide(a, b);
	CHECK(glm::distance(a / b, r) < 1e-8f);
}

TEST_CASE("Component-wise lerp")
{
	glm::vec2 a(2, 4);
	glm::vec2 b(3, 5);
	float t = 0.25;
	glm::vec2 r = componentWiseLerp(a, b, t);
	CHECK(glm::distance(glm::mix(a, b, t), r) < 1e-8f);
}

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