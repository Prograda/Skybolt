/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetSubdivisionPredicate.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgMathHelpers.h"

#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

bool PlanetSubdivisionPredicate::operator()(const Box2d& bounds, const QuadTreeTileKey& key)
{
	if (key.level >= maxLevel)
	{
		return false;
	}

	Box2d latLonBounds(math::vec2SwapComponents(bounds.minimum), math::vec2SwapComponents(bounds.maximum));

	osg::Vec2d latLon = nearestPointInSolidBox(observerLatLon, latLonBounds);

	osg::Vec3d tileP = llaToGeocentric(latLon, 0, planetRadius);
	osg::Vec3d observerP = llaToGeocentric(observerLatLon, std::max(1.0, observerAltitude), planetRadius);

	osg::Vec3d dir = observerP - tileP;
	double distance = dir.normalize();
	tileP.normalize();

	float cosElevation = dir * tileP;
	bool visible = (cosElevation > 0.0f); // TODO: tune

	if (visible)
	{
		double tileSize = planetRadius / std::pow(2, key.level);
		double projectedSize = tileSize / std::max(0.01, distance);
		return projectedSize > glm::mix(0.4f, 0.1f, cosElevation); // TODO: tune
	}

	return false;
}

osg::Vec2d PlanetSubdivisionPredicate::nearestPointInSolidBox(const osg::Vec2d& point, const Box2d& bounds) const
{
	// Handle longitude wrap around
	osg::Vec2d wrappedPoint = point;
	double centerLon = bounds.center().y();

	double dist = std::abs(point.y() - centerLon);
	double candidateDist = std::abs(point.y() - math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y() -= math::twoPiD();
		dist = candidateDist;
	}
	candidateDist = std::abs(point.y() + math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y() += math::twoPiD();
	}

	// Now find nearest point
	return osg::Vec2d(math::clamp(wrappedPoint.x(), bounds.minimum.x(), bounds.maximum.x()),
		math::clamp(wrappedPoint.y(), bounds.minimum.y(), bounds.maximum.y()));
}

} // namespace vis
} // namespace skybolt
