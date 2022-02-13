/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MathUtility.h"

namespace skybolt {

namespace math {

	glm::vec2 vec2Rotate(const glm::vec2 &v, float theta)
	{
		float cs = cos(theta);
		float sn = sin(theta);

		glm::vec2 r;
		r.x = v.x * cs - v.y * sn;
		r.y = v.x * sn + v.y * cs;
		return r;
	}

	void getOrthonormalBasis(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent)
	{
		float d = glm::dot(normal, vec3UnitY());
		if (d > -0.95f && d < 0.95f)
			bitangent = glm::cross(normal, vec3UnitY());
		else
			bitangent = glm::cross(normal, vec3UnitZ());
		bitangent = glm::normalize(bitangent);
		tangent = glm::cross(bitangent, normal);
	}

	glm::quat quatFromEuler(const glm::vec3 &eulerRPY)
	{
		return glm::angleAxis(eulerRPY.z, vec3UnitZ()) * glm::angleAxis(eulerRPY.y, vec3UnitY()) * glm::angleAxis(eulerRPY.x, vec3UnitX());
	}

	glm::dquat quatFromEuler(const glm::dvec3 &eulerRPY)
	{
		return glm::angleAxis(eulerRPY.z, dvec3UnitZ()) * glm::angleAxis(eulerRPY.y, dvec3UnitY()) * glm::angleAxis(eulerRPY.x, dvec3UnitX());
	}

	template <typename T>
	glm::tvec3<T> eulerFromQuatT(const glm::tquat<T>& quat)
	{
		glm::tvec3<T> euler;

		glm::tvec3<T> forward = quat * glm::tvec3<T>(1, 0, 0);
		glm::tvec3<T> up = quat * glm::tvec3<T>(0, 0, -1);
		// TODO: fix singularity when forwardXY is zero
		glm::tvec3<T> fowardXY = glm::normalize(glm::tvec3<T>(forward.x, forward.y, 0));

		euler.y = angleBetween(fowardXY, forward) * glm::sign(glm::cross(fowardXY, glm::cross(fowardXY, forward)).z); // pitch
		euler.z = angleBetween(glm::tvec3<T>(1, 0, 0), fowardXY); // yaw
		if (glm::cross(glm::tvec3<T>(1, 0, 0), fowardXY).z < 0.0f)
		{
			euler.z *= -1.0f;
		}

		glm::tvec3<T> upVertPlane = glm::angleAxis(euler.z, glm::tvec3<T>(0, 0, 1)) * glm::angleAxis(euler.y, glm::tvec3<T>(0, 1, 0)) * glm::tvec3<T>(0, 0, -1);
		euler.x = angleBetween(upVertPlane, up) * glm::sign(glm::dot(forward, glm::cross(upVertPlane, up))); // roll

		return euler;
	}

	glm::vec3 eulerFromQuat(const glm::quat& quat)
	{
		return eulerFromQuatT(quat);
	}

	glm::dvec3 eulerFromQuat(const glm::dquat& quat)
	{
		return eulerFromQuatT(quat);
	}

	int nextPow2(int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;

		return v;
	}

	const glm::quat& quatIdentity()
	{
		static glm::quat q(1, 0, 0, 0);
		return q;
	}

	const glm::dquat& dquatIdentity()
	{
		static glm::dquat q(1, 0, 0, 0);
		return q;
	}

	const glm::dmat4& dmat4Identity()
	{
		static glm::dmat4 m(1.0);
		return m;
	}

	const glm::vec3& vec3Zero()
	{
		static glm::vec3 r(0, 0, 0);
		return r;
	}

	const glm::vec3& vec3UnitX()
	{
		static glm::vec3 r(1, 0, 0);
		return r;
	}

	const glm::vec3& vec3UnitY()
	{
		static glm::vec3 r(0, 1, 0);
		return r;
	}

	const glm::vec3& vec3UnitZ()
	{
		static glm::vec3 r(0, 0, 1);
		return r;
	}

	const glm::dvec3& dvec3Zero()
	{
		static glm::dvec3 r(0, 0, 0);
		return r;
	}

	const glm::dvec3& dvec3UnitX()
	{
		static glm::dvec3 r(1, 0, 0);
		return r;
	}

	const glm::dvec3& dvec3UnitY()
	{
		static glm::dvec3 r(0, 1, 0);
		return r;
	}

	const glm::dvec3& dvec3UnitZ()
	{
		static glm::dvec3 r(0, 0, 1);
		return r;
	}

	float piF()
	{
		static float r = 3.14159265f;
		return r;
	}

	float halfPiF()
	{
		static float r = piF() * 0.5f;
		return r;
	}

	float twoPiF()
	{
		static float r = piF() * 2.0f;
		return r;
	}

	float degToRadF()
	{
		static float r = 0.0174532925f;
		return r;
	}

	float radToDegF()
	{
		static float r = 1.0f / degToRadF();
		return r;
	}

	double degToRadD()
	{
		static double r = 0.01745329251994329576923690768489;
		return r;
	}
	double radToDegD()
	{
		static double r = 1.0 / degToRadD();
		return r;
	}

	double piD()
	{
		static double r = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825;
		return r;
	}

	double halfPiD()
	{
		static double r = piD() * 0.5;
		return r;
	}

	double twoPiD()
	{
		static double r = piD() * 2.0;
		return r;
	}

	float posInfinity()
	{
		static float r = std::numeric_limits<float>::infinity();
		return r;
	}

	float negInfinity()
	{
		static float r = -std::numeric_limits<float>::infinity();
		return r;
	}

	std::string toString(const glm::dvec3& v)
	{
		return "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")";
	}

	std::string toString(const glm::dquat& v)
	{
		return "(" + std::to_string(v.w) + ",(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "))";
	}

} // namespace math
} // namespace skybolt