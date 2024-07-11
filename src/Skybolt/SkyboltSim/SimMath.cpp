/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SimMath.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <limits>

namespace skybolt {
namespace sim {

void getOrthonormalBasis(const Vector3 &normal, Vector3 &tangent, Vector3 &bitangent)
{
	double d = glm::dot(normal, Vector3(0,1,0));
	if (d > -0.95f && d < 0.95f)
		bitangent = glm::cross(normal, Vector3(0,1,0));
	else
		bitangent = glm::cross(normal, Vector3(0,0,1));
	bitangent = glm::normalize(bitangent);
	tangent = glm::cross(bitangent, normal);
}

Matrix3 getOrientationFromDirection(const Vector3& direction)
{
	Vector3 tangent, bitangent;
	getOrthonormalBasis(direction, tangent, bitangent);
	return Matrix3(direction, tangent, bitangent);
}

double posInfinity()
{
	constexpr auto r = std::numeric_limits<double>::infinity();
	return r;
}

double negInfinity()
{
	constexpr auto r = -std::numeric_limits<double>::infinity();
	return r;
}

} // namespace sim
} // namespace skybolt