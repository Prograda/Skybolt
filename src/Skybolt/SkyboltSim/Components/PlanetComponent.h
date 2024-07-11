/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/PlanetAltitudeProvider.h"
#include <SkyboltSim/Physics/Atmosphere.h>
#include "SkyboltSim/SkyboltSimFwd.h"

#include <variant>

namespace skybolt {
namespace sim {

struct PlanetComponent : public Component
{
public:
	explicit PlanetComponent(double radius) :
		radius(radius)
	{
	}

	const double radius;

	std::shared_ptr<PlanetAltitudeProvider> altitudeProvider; //!< Null if the planet has no terrain
	std::optional<Atmosphere> atmosphere;
};

} // namespace sim
} // namespace skybolt