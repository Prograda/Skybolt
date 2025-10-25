/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <optional>
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

template <class VecT>
VecT vec2SwapComponents(const VecT& v)
{
	return VecT(v[1], v[0]);
}

template <class VecT>
VecT vec2Rotate(const VecT &v, typename VecT::value_type theta)
{
	typename VecT::value_type cs = cos(theta);
	typename VecT::value_type sn = sin(theta);

	return VecT(
		v.x * cs - v.y * sn,
		v.x * sn + v.y * cs
	);
}

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
//! @param tangent is a vector in the tangent plane (perpendicular to the normal and the bitangent)
//! @param outTangent is a normalized output tangent vector
//! @param outBitangent is a normalized output tangent vector
template <typename VecT>
void getOrthonormalBasis(const VecT& normal, const VecT& tangent, VecT& outTangent, VecT& outBitangent)
{
	outBitangent = glm::normalize(
		glm::cross(normal, tangent)
	);
	outTangent = glm::cross(outBitangent, normal);
}

//! @param normal must be normalized.
//! @param outTangent is a normalized output tangent vector
//! @param outBitangent is a normalized output tangent vector
template <typename VecT>
void getOrthonormalBasis(const VecT &normal, VecT &outTangent, VecT &outBitangent)
{
	constexpr VecT unitY(0, 1, 0);
	constexpr VecT unitZ(0, 0, 1);
	auto d = glm::dot(normal, unitY);
	bool parallelWithY = std::abs(d) > typename VecT::value_type(0.95);
	VecT tangent = parallelWithY ? unitZ : unitY;

	getOrthonormalBasis(normal, tangent, outTangent, outBitangent);
}

template <typename VecT>
inline std::optional<VecT> normalizeSafe(const VecT& v)
{
	double length = glm::length(v);
	if (length < std::numeric_limits<typename VecT::value_type>::epsilon())
	{
		return std::nullopt;
	}
	return v / length;
}

//! @returns angle between two normalized vectors
template <typename T>
typename T::value_type angleBetweenDirections(const T &v0, const T &v1)
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

//! reranges value from range [originalMin, originalMax] to [0, 1]
template<typename T>
float rerangeNormalized(T originalValue, T originalMin, T originalMax)
{
	return (((originalValue - originalMin) / (originalMax - originalMin)));
}

//! reranges value from range [originalMin, originalMax] to [newMin, newMax]
template<typename T>
float rerange(T originalValue, T originalMin, T originalMax, T newMin, T newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

} // namespace math
} // namespace skybolt
