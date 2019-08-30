/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <string>

namespace skybolt {
namespace math {

template<class T>
T clamp(T v, T a, T b)
{
	return std::max(a, std::min(v, b));
}

template<class T>
T lerp(T x, T y, T t)
{
	return x + t*(y-x);
}

glm::vec2 vec2Rotate(const glm::vec2 &v, float theta);

//! @param normal must be normalized.
//! Output vectors are normalized
void getOrthonormalBasis(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);

template <typename T>
T angleBetween(const glm::detail::tvec3<T> &v0, const glm::detail::tvec3<T> &v1)
{
	T dot = glm::dot(v0, v1);
	dot = glm::clamp(dot, T(-1), T(1));
	return glm::acos(dot);
}

//! Euler in radians. Component order roll, pitch, yaw. Rotation order yaw, pitch, roll.
glm::quat quatFromEuler(const glm::vec3 &eulerRPY);
glm::dquat quatFromEuler(const glm::dvec3 &eulerRPY);

glm::vec3 eulerFromQuat(const glm::quat& quat);
glm::dvec3 eulerFromQuat(const glm::dquat& quat);

template<typename T>
T lerpShortestRotation(T a, T b, T weight)
{
	if (std::abs(a - b) > piD())
	{
		if (a < b)
			a += twoPiD();
		else
			b += twoPiD();
	}
	return a + weight * (b - a);
}

int nextPow2(int v);

const glm::quat& quatIdentity();
const glm::dquat& dquatIdentity();

const glm::vec3& vec3Zero();
const glm::vec3& vec3UnitX();
const glm::vec3& vec3UnitY();
const glm::vec3& vec3UnitZ();

const glm::dvec3& dvec3Zero();
const glm::dvec3& dvec3UnitX();
const glm::dvec3& dvec3UnitY();
const glm::dvec3& dvec3UnitZ();

float piF();
float halfPiF();
float twoPiF();

float degToRadF();
float radToDegF();

double degToRadD();
double radToDegD();

double piD();
double halfPiD();
double twoPiD();

float posInfinity();
float negInfinity();

std::string toString(const glm::dvec3& v);
std::string toString(const glm::dquat& v);

} // namespace math
} // namespace skybolt
