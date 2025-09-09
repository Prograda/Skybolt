/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"

namespace skybolt {
namespace sim {

class ShipWakeComponent : public Component
{
public:
	enum class Type
	{
		SHIP_WAKE,
		ROTOR_WASH
	};

	Type type;
	float bowWakeForwardOffset; //!< Distance at which wake starts relative to the entity's origin, along the +x axis
	float sternWakeForwardOffset;
	float startWidth;
	float length;
	float spreadAngularWidth; //!< Angular width in radians of the wave spread. A value of zero maintains startWidth.
};

} // namespace sim
} // namespace skybolt