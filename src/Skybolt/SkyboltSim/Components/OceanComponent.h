/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/OceanSurfaceSampler.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt::sim {

struct OceanComponent : public Component
{
public:
	double waveHeight = 1;
	double windVelocityHeading = 0;
	OceanSurfaceSamplerPtr surfaceSampler = std::make_shared<PlanarOceanSurfaceSampler>(); //!< Never null
};

SKYBOLT_REFLECT_EXTERN(OceanComponent)

} // namespace skybolt::sim