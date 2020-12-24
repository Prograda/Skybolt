/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/PlanetAltitudeProvider.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

struct PlanetComponent : public sim::Component
{
	PlanetComponent(double radius, bool hasOcean, const std::shared_ptr<AsyncPlanetAltitudeProvider>& altitudeProvider) :
		radius(radius),
		hasOcean(hasOcean),
		altitudeProvider(altitudeProvider)
	{
	}

	const double radius;
	const bool hasOcean;
	const std::shared_ptr<AsyncPlanetAltitudeProvider> altitudeProvider;
};

} // namespace sim
} // namespace skybolt