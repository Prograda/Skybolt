/* Copyright Matthew Reid
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

template <class T>
T vec2SwapComponents(const T& v)
{
	return T(v[1], v[0]);
}

glm::vec2 vec2Rotate(const glm::vec2 &v, float theta);

template <typename T>
constexpr size_t componentCount(const typename glm::tvec2<T>& v) {return 2; }

template <typename T>
constexpr size_t componentCount(const typename glm::tvec3<T>& v) { return 3; }

template <typename T>
constexpr size_t componentCount(const typename glm::tvec4<T>& v) { return 4; }

template <typename T, typename V>
T componentWiseMultiply(const V& s, const T& v)
{
	T r;
	for (int i = 0; i < componentCount(v); ++i)
	{
		r[i] = s * v[i];
	}
	return r;
}

template <typename T>
T componentWiseMultiply(const T& a, const T& b)
{
	T r;
	for (int i = 0; i < componentCount(a); ++i)
	{
		r[i] = a[i] * b[i];
	}
	return r;
}

template <typename T>
T componentWiseDivide(const T& a, const T& b)
{
	T r;
	for (int i = 0; i < componentCount(a); ++i)
	{
		r[i] = a[i] / b[i];
	}
	return r;
}

template <typename T, typename V>
T componentWiseLerp(const T& a, const T& b, V t)
{
	return a + componentWiseMultiply(t, (b - a));
}

//! @param normal must be normalized.
//! Output vectors are normalized
void getOrthonormalBasis(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);

template <typename T>
typename T::value_type angleBetween(const T &v0, const T &v1)
{
	using ValT = typename T::value_type;
	ValT dot = glm::dot(v0, v1);
	dot = glm::clamp(dot, ValT(-1), ValT(1));
	return glm::acos(dot);
}

//! Euler in radians. Component order roll, pitch, yaw. Rotation order yaw, pitch, roll.
glm::quat quatFromEuler(const glm::vec3 &eulerRPY);
glm::dquat quatFromEuler(const glm::dvec3 &eulerRPY);

glm::vec3 eulerFromQuat(const glm::quat& quat);
glm::dvec3 eulerFromQuat(const glm::dquat& quat);

int nextPow2(int v);

const glm::quat& quatIdentity();
const glm::dquat& dquatIdentity();

const glm::dmat4& dmat4Identity();

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

//! Like c++ fmod except that it doesn't flip behavior for negative numbers, i.e fmod(-5, 6) = 1
template<typename T>
T fmodNeg(T a, T b)
{
	// From https://stackoverflow.com/questions/1082917/mod-of-negative-number-is-melting-my-brain
	return a - b * floor(a / b);
}

template<typename T>
T normalizeAngleTwoPi(T a)
{
	return fmodNeg(a, T(twoPiD()));
}

template<typename T>
T calcSmallestAngleFromTo(T a, T b)
{
	a = normalizeAngleTwoPi(a);
	b = normalizeAngleTwoPi(b);
	if (std::abs(a - b) > piD())
	{
		if (a < b)
			a += T(twoPiD());
		else
			b += T(twoPiD());
	}

	return b - a;
}

template<typename T>
T lerpShortestRotation(T a, T b, T weight)
{
	if (std::abs(a - b) > piD())
	{
		if (a < b)
			a += T(twoPiD());
		else
			b += T(twoPiD());
	}
	return a + weight * calcSmallestAngleFromTo(a, b);
}

} // namespace math
} // namespace skybolt
