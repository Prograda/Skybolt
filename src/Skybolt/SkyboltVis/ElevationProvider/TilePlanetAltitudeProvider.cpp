/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TilePlanetAltitudeProvider.h"
#include "HeightMapElevationProvider.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/GeoImageHelpers.h"
#include <SkyboltSim/Spatial/GreatCircle.h>

namespace skybolt {
namespace vis {

static vis::Box2f toBox2f(const TilePlanetAltitudeProvider::LatLonBounds& b)
{
	return vis::Box2f(osg::Vec2f(b.minimum.x(), b.minimum.y()), osg::Vec2f(b.maximum.x(), b.maximum.y()));
}

TilePlanetAltitudeProvider::TilePlanetAltitudeProvider(const TileSourcePtr& tileSource, int maxLod) :
	mTileSource(tileSource),
	mMaxLod(maxLod),
	mTileImageCache(1024)
{
	assert(mTileSource);
}

double TilePlanetAltitudeProvider::getAltitude(const sim::LatLon& position) const
{
	QuadTreeTileKey highestLodKey = getKeyAtLevelIntersectingLonLatPoint(mMaxLod, LatLonVec2Adapter(position));
	std::optional<TilePlanetAltitudeProvider::TileImage> result = findHighestLodTile(highestLodKey);
	if (!result)
	{
		return 0.0;
	}

	std::optional<HeightMapElevationRerange> rerange = getHeightMapElevationRerange(*result->image);
	if (!rerange)
	{
		return 0.0;
	}

	vis::HeightMapElevationProvider provider(result->image, *rerange, toBox2f(getKeyLatLonBounds<LatLonVec2Adapter>(result->key)));
	return provider.get(position.lat, position.lon);
}

std::optional<double> TilePlanetAltitudeProvider::tryGetAltitude(const sim::LatLon& position) const
{
	QuadTreeTileKey highestLodKey = getKeyAtLevelIntersectingLonLatPoint(mMaxLod, LatLonVec2Adapter(position));

	TileImage result;
	bool success;
	{
		std::scoped_lock<std::mutex> lock(mTileImageCacheMutex);
		success = mTileImageCache.get(highestLodKey, result);
	}
	if (success)
	{
		std::optional<HeightMapElevationRerange> rerange = getHeightMapElevationRerange(*result.image);
		if (!rerange)
		{
			return {};
		}

		vis::HeightMapElevationProvider provider(result.image, *rerange,  toBox2f(getKeyLatLonBounds<LatLonVec2Adapter>(result.key)));
		return provider.get(position.lat, position.lon);
	}

	return {};
}

std::optional<TilePlanetAltitudeProvider::TileImage> TilePlanetAltitudeProvider::findHighestLodTile(const QuadTreeTileKey& highestLodKey) const
{
	// If tile image exists in the cache, use it
	TileImage result;
	{
		std::scoped_lock<std::mutex> lock(mTileImageCacheMutex);
		if (mTileImageCache.get(highestLodKey, result))
		{
			return result;
		}
	}

	int level = highestLodKey.level;
	QuadTreeTileKey key = highestLodKey;
	while (level >= 0)
	{
		osg::ref_ptr<osg::Image> image = mTileSource->createImage(key, [] {return false;});
		if (image)
		{
			// Found image
			TileImage result;
			result.image = image;
			result.key = key;

			// Add to cache at highest LOD level
			{
				std::scoped_lock<std::mutex> lock(mTileImageCacheMutex);
				mTileImageCache.putSafe(highestLodKey, result);

				// Add to cache at lower LOD level so if the highest level has a cache miss
				// we can still potentially avoid reloading the image
				if (level != highestLodKey.level)
				{
					mTileImageCache.putSafe(key, result);
				}
			}

			return result;
		}
		--level;
		key = createAncestorKey(highestLodKey, level);
	}

	return std::nullopt;
}

TileAsyncPlanetAltitudeProvider::TileAsyncPlanetAltitudeProvider(px_sched::Scheduler* scheduler, const TileSourcePtr& tileSource, int maxLod) :
	mScheduler(scheduler),
	mProvider(std::make_unique<TilePlanetAltitudeProvider>(tileSource, maxLod))
{
	assert(mScheduler);
}

std::optional<double> TileAsyncPlanetAltitudeProvider::getAltitudeOrRequestLoad(const sim::LatLon& position) const
{
	auto result = mProvider->tryGetAltitude(position);
	if (!result)
	{
		if (mScheduler->hasFinished(mLoadingTaskSync))
		{
			mScheduler->run([=]() {
				mProvider->getAltitude(position);
			}, &mLoadingTaskSync);
		}
	}
	return result;
}

} // namespace vis
} // namespace skybolt