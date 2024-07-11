/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/MathUtility.h>
#include <LinearMath/btQuaternion.h>

namespace skybolt {
namespace sim {

inline glm::dvec3 toGlmDvec3(const btVector3& v)
{
	return glm::dvec3(v.x(), v.y(), v.z());
}

inline btVector3 toBtVector3(const glm::dvec3& v)
{
	return btVector3(v.x, v.y, v.z);
}

inline glm::dquat toGlmDquat(const btQuaternion& q)
{
	return glm::dquat(q.w(), q.x(), q.y(), q.z());
}

inline btQuaternion toBtQuaternion(const glm::dquat& q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

} // namespace sim
} // namespace skybolt