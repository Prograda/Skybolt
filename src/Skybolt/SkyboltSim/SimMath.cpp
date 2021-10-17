/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SimMath.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <limits>

namespace skybolt {
namespace sim {

void getOrthonormalBasis(const Vector3 &normal, Vector3 &tangent, Vector3 &binormal)
{
	Real d = (Real)glm::dot(normal, Vector3(0,1,0));
	if (d > -0.95f && d < 0.95f)
		binormal = glm::cross(normal, Vector3(0,1,0));
	else
		binormal = glm::cross(normal, Vector3(0,0,1));
	binormal = glm::normalize(binormal);
	tangent = glm::cross(binormal, normal);
}

Matrix3 getOrientationFromDirection(const Vector3& direction)
{
	Vector3 tangent, binormal;
	getOrthonormalBasis(direction, tangent, binormal);
	return Matrix3(direction, tangent, binormal);
}

DistReal posInfinity()
{
	auto r = std::numeric_limits<DistReal>::infinity();
	return r;
}

DistReal negInfinity()
{
	auto r = -std::numeric_limits<DistReal>::infinity();
	return r;
}

} // namespace sim
} // namespace skybolt