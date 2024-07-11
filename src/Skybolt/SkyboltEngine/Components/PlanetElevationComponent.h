/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

struct PlanetElevationComponent : public sim::Component
{
	vis::TileSourcePtr tileSource;
	int elevationMaxLodLevel;
	bool heightMapTexelsOnTileEdge;
};

} // namespace skybolt