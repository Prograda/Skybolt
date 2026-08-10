/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TilePlanetAltitudeProvider.h"
#include "ElevationImageMetadata.h"
#include "HeightMapElevationProvider.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h"
#include <SkyboltSim/Spatial/GreatCircle.h>

namespace skybolt {
namespace vis {

BlockingTilePlanetAltitudeProvider::BlockingTilePlanetAltitudeProvider(const TileSourcePtr& tileSource, int maxLod) :
	mTileSource(tileSource),
	mMaxLod(maxLod),
	mTileImageCache(1024)
{
	assert(mTileSource);
}

inline glm::dvec2 toLonLatVec(const sim::LatLon& p)
{
	return glm::dvec2(p.lon, p.lat);
}

BlockingTilePlanetAltitudeProvider::AltitudeResult BlockingTilePlanetAltitudeProvider::getAltitude(const sim::LatLon& position) const
{
	std::optional<TileImage> tile;
	QuadTreeTileKey highestLodKey = getKeyAtLevelIntersectingLonLatPoint(mMaxLod, toLonLatVec(position));

	tile = findTile(highestLodKey);
	if (!tile)
	{
		// Load tile at highest available LOD level
		for (int lod = mMaxLod; lod >= 0; lod--)
		{
			QuadTreeTileKey key = key = createAncestorKey(highestLodKey, lod);
			tile = loadTile(key);
			if (tile)
			{
				// Add tile at highest LOD key so we can find it quickly next time from highestLodKey
				addTileToCache(*tile, highestLodKey);
				break;
			}
		}
	}

	if (!tile)
	{
		return AltitudeResult::provisionalValue(0.0);
	}
	auto metadata = getElevationImageMetadataRequired(*tile->image);
	vis::HeightMapElevationProvider provider(tile->image, metadata.rerange, getKeyLatLonBounds<glm::dvec2>(tile->key));
	return AltitudeResult::finalValue(provider.get(float(position.lat), float(position.lon)));
}

std::optional<BlockingTilePlanetAltitudeProvider::TileImage> BlockingTilePlanetAltitudeProvider::findTile(const QuadTreeTileKey& key) const
{
	TileImage result;
	std::scoped_lock<std::mutex> lock(mTileImageCacheMutex);
	if (mTileImageCache.get(key, result))
	{
		return result;
	}
	return std::nullopt;
}

std::optional<BlockingTilePlanetAltitudeProvider::TileImage> BlockingTilePlanetAltitudeProvider::loadTile(const QuadTreeTileKey& key) const
{
	int level = key.level;
	ImagePtr image = mTileSource->createImage(key, [] {return false;});
	if (image)
	{
		// Found image
		TileImage result;
		result.image = image;
		result.key = key;
		return result;
	}
	return std::nullopt;
}


void BlockingTilePlanetAltitudeProvider::addTileToCache(const TileImage& image, const QuadTreeTileKey& key) const
{
	std::scoped_lock<std::mutex> lock(mTileImageCacheMutex);
	mTileImageCache.putSafe(key, image);
}

NonBlockingTilePlanetAltitudeProvider::NonBlockingTilePlanetAltitudeProvider(px_sched::Scheduler* scheduler, const TileSourcePtr& tileSource, int maxLod) :
	BlockingTilePlanetAltitudeProvider(tileSource, maxLod),
	mScheduler(scheduler)
{
	assert(mScheduler);
	assert(mTileSource);
}

BlockingTilePlanetAltitudeProvider::AltitudeResult NonBlockingTilePlanetAltitudeProvider::getAltitude(const sim::LatLon& position) const
{
	std::optional<TileImage> highestLodTile;
	for (int lod = 0; lod <= mMaxLod; lod++)
	{
		QuadTreeTileKey key = getKeyAtLevelIntersectingLonLatPoint(lod, toLonLatVec(position));
		std::optional<TileImage> tile = findTile(key);
		if (tile)
		{
			highestLodTile = tile;
		}
		else
		{
			requestLoadTileAndAddToCache(key);
			break;
		}
	}

	if (!highestLodTile)
	{
		return AltitudeResult::provisionalValue(0.0);
	}

	auto metadata = getElevationImageMetadataRequired(*highestLodTile->image);
	vis::HeightMapElevationProvider provider(highestLodTile->image, metadata.rerange, getKeyLatLonBounds<glm::dvec2>(highestLodTile->key));

	AltitudeResult result;
	result.altitude = provider.get(position.lat, position.lon);
	result.provisional = (highestLodTile->key.level < mMaxLod);
	return result;
}

void NonBlockingTilePlanetAltitudeProvider::requestLoadTileAndAddToCache(const QuadTreeTileKey& key) const
{
	if (mScheduler->hasFinished(mLoadingTaskSync))
	{
		mScheduler->run([=]() {
			if (std::optional<TileImage> image = loadTile(key); image)
			{
				addTileToCache(*image, key);
			}
		}, &mLoadingTaskSync);
	}
}

} // namespace vis
} // namespace skybolt