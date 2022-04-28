/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "QuadTreeTileLoader.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/OsgBox2.h"
#include <SkyboltCommon/Math/QuadTree.h>
#include <osg/Vec2d>

namespace skybolt {
namespace vis {

struct PlanetSubdivisionPredicate : public QuadTreeSubdivisionPredicate
{
	~PlanetSubdivisionPredicate() override = default;

	bool operator()(const Box2d& bounds, const skybolt::QuadTreeTileKey& key) override;

	std::vector<TileSourcePtr> tileSources; //!< tileSources are queried to see if children exist at each level
	osg::Vec2d observerLatLon;
	double observerAltitude;
	double planetRadius;

private:
	// TODO: handle longitude wrap around
	osg::Vec2d nearestPointInSolidBox(const osg::Vec2d& point, const Box2d& bounds) const;
};

} // namespace vis
} // namespace skybolt
