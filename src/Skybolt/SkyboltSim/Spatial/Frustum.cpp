/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Frustum.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

Vector3 transformToScreenSpace(const Frustum& frustum, const Vector3& point)
{
	Vector3 p = glm::inverse(frustum.orientation) * (point - frustum.origin);
	if (p.x == 0.0)
	{
		return math::dvec3Zero();
	}

	return Vector3(
		p.y / (std::tan(frustum.fieldOfViewHorizontal * 0.5) * p.x),
		-p.z / (std::tan(frustum.fieldOfViewVertical * 0.5) * p.x),
		p.x);
}

} // namespace sim
} // namespace skybolt