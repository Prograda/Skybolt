/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class Motion : public Component
{
public:
	Vector3 linearVelocity = math::dvec3Zero();
	Vector3 angularVelocity = math::dvec3Zero(); //!< angular velocity in world axes, not body axes
};

SKYBOLT_REFLECT_EXTERN(Motion)

} // namespace sim
} // namespace skybolt