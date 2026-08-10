/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "QuadTreeTileLoader.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltCommon/Math/QuadTree.h>

namespace skybolt {
namespace vis {

struct PlanetSubdivisionPredicate : public QuadTreeSubdivisionPredicate
{
	~PlanetSubdivisionPredicate() override = default;

	bool operator()(const Box2d& bounds, const skybolt::QuadTreeTileKey& key, const TileImages* images) override;

	std::vector<TileSourcePtr> tileSources; //!< tileSources are queried to see if children exist at each level
	glm::dvec2 observerLatLon;
	double observerAltitude;
	double planetRadius;

private:
	// TODO: handle longitude wrap around
	glm::dvec2 nearestPointInSolidBox(const glm::dvec2& point, const Box2d& bounds) const;
};

} // namespace vis
} // namespace skybolt
