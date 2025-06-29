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

Matrix3 getOrientationFromDirection(const Vector3& direction)
{
	Vector3 tangent, bitangent;
	math::getOrthonormalBasis(direction, tangent, bitangent);
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