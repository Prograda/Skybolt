/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"

namespace skybolt {
namespace sim {

class Positionable
{
public:
	virtual ~Positionable() = default;
	virtual Vector3 getPosition() const = 0;
	virtual Quaternion getOrientation() const = 0;
};

} // namespace sim
} // namespace skybolt