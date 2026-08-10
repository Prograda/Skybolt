/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetSubdivisionPredicate.h"
#include "SkyboltVis/Elevation/ElevationBounds.h"
#include "SkyboltVis/Elevation/ElevationImageMetadata.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetTileImagesLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h"

#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Spatial/Geocentric.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

static bool hasAnyChildren(const std::vector<TileSourcePtr>& tileSources, const QuadTreeTileKey& key)
{
	for (const auto& tileSource : tileSources)
	{
		if (tileSource->hasAnyChildren(key))
		{
			return true;
		}
	}
	return false;
}

bool PlanetSubdivisionPredicate::operator()(const Box2d& bounds, const QuadTreeTileKey& key, const TileImages* images)
{
	// Don't subdivide if the tile is not loaded yet.
	if (!images)
	{
		return false;
	}

	// Don't subdivide if the are no children available in the tile source data.
	if (!hasAnyChildren(tileSources, key))
	{
		return false;
	}

	const auto& tileImages = static_cast<const PlanetTileImages&>(*images);
	auto metadata = getElevationImageMetadataRequired(*tileImages.heightMapImage.image);
	const ElevationBounds& elevationBounds = metadata.elevationBounds;

	Box2d latLonBounds(math::vec2SwapComponents(bounds.minimum), math::vec2SwapComponents(bounds.maximum));

	glm::dvec2 latLon = nearestPointInSolidBox(observerLatLon, latLonBounds);
	double altitude = std::clamp(observerAltitude, double(elevationBounds.x), double(elevationBounds.y));

	glm::dvec3 observerPosition = llaToGeocentric(sim::LatLonAlt(observerLatLon.x, observerLatLon.y, std::max(1.0, observerAltitude)), planetRadius);

	glm::dvec3 tileNearestPoint = llaToGeocentric(sim::LatLonAlt(latLon.x, latLon.y, altitude), planetRadius);
	double distanceToTileNearestPoint = glm::distance(observerPosition, tileNearestPoint);

	glm::dvec3 tileNearestPointAtLowestAltitude = llaToGeocentric(sim::LatLonAlt(latLon.x, latLon.y, 0), planetRadius + elevationBounds.x);

	glm::dvec3 directionFromTileNearestPointAtLowestAltitudeToObserver = glm::normalize(observerPosition - tileNearestPointAtLowestAltitude);

	double cosElevation = glm::dot(directionFromTileNearestPointAtLowestAltitudeToObserver, glm::normalize(tileNearestPointAtLowestAltitude));
	bool visible = (cosElevation > 0.0f);

	if (visible)
	{
		double tileSize = planetRadius / std::pow(2, key.level);
		double projectedSize = tileSize / std::max(0.01, distanceToTileNearestPoint);
		return projectedSize > glm::mix(0.4, 0.1, cosElevation); // TODO: tune
	}

	return false;
}

glm::dvec2 PlanetSubdivisionPredicate::nearestPointInSolidBox(const glm::dvec2& point, const Box2d& bounds) const
{
	// Handle longitude wrap around
	glm::dvec2 wrappedPoint = point;
	double centerLon = bounds.center().y;

	double dist = std::abs(point.y - centerLon);
	double candidateDist = std::abs(point.y - math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y -= math::twoPiD();
		dist = candidateDist;
	}
	candidateDist = std::abs(point.y + math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y += math::twoPiD();
	}

	// Now find nearest point
	return glm::dvec2(math::clamp(wrappedPoint.x, bounds.minimum.x, bounds.maximum.x),
		math::clamp(wrappedPoint.y, bounds.minimum.y, bounds.maximum.y));
}

} // namespace vis
} // namespace skybolt
