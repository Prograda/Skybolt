/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"

namespace skybolt {
namespace sim {

struct Frustum
{
	Vector3 origin;
	Quaternion orientation;
	double fieldOfViewHorizontal; //!< Field of view angle in radians about the Z axis
	double fieldOfViewVertical; //!< Field of view angle in radians about the Y axis
};

//! Transforms a point to screen space.
//! @return {right coordinate in range [-1,1], up coordinate in range [-1,1], depth coordinate in world units}
//! NOTE: the depth coordinate != euclidean distance. Depth is the projected distance along forward axis.
Vector3 transformToScreenSpace(const Frustum& frustum, const Vector3& point);

} // namespace sim
} // namespace skybolt