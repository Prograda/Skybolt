/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetAltitudeProvider.h"
#include "HeightmapElevationProvider.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/GeoImageHelpers.h"
#include <SkyboltSim/Spatial/GreatCircle.h>

namespace skybolt {
namespace vis {

static vis::Box2f toBox2f(const PlanetAltitudeProvider::LatLonBounds& b)
{
	return vis::Box2f(osg::Vec2f(b.minimum.x(), b.minimum.y()), osg::Vec2f(b.maximum.x(), b.maximum.y()));
}

PlanetAltitudeProvider::PlanetAltitudeProvider(const TileSourcePtr& tileSource, int maxLod) :
	mTileSource(tileSource),
	mMaxLod(maxLod),
	mTileImageCache(1024)
{
	assert(mTileSource);
}

double PlanetAltitudeProvider::getAltitude(const sim::LatLon& position) const
{
	boost::optional<PlanetAltitudeProvider::TileImage> result = findHighestLodTileIntersectingPoint(position);
	if (!result)
	{
		return 0.0;
	}

	vis::HeightmapElevationProvider provider(result->image, toBox2f(getKeyLatLonBounds<LatLonVec2Adapter>(result->key)));
	return -provider.get(position.lat, position.lon);
}

boost::optional<PlanetAltitudeProvider::TileImage> PlanetAltitudeProvider::findHighestLodTileIntersectingPoint(const sim::LatLon& position) const
{
	// Load image at highest available lod
	QuadTreeTileKey highestLodKey = getKeyAtLevelIntersectingLonLatPoint(mMaxLod, LatLonVec2Adapter(position));

	int level = highestLodKey.level;
	QuadTreeTileKey key = highestLodKey;
	while (level >= 0)
	{
		// If tile image exists in the cache, use it
		TileImage result;
		if (mTileImageCache.get(highestLodKey, result))
		{
			return result;
		}

		osg::ref_ptr<osg::Image> image = mTileSource->createImage(key, [] {return false;});
		if (image)
		{
			// Found image
			TileImage result;
			result.image = image;
			result.key = key;

			// Add to cache at highest LOD level
			mTileImageCache.putOrGet(highestLodKey, result);

			// Add to cache at lower LOD level so if the highest level has a cache miss
			// we can still potentially avoid reloading the image
			if (level != highestLodKey.level)
			{
				mTileImageCache.putOrGet(key, result);
			}

			return result;
		}
		--level;
		key = createAncestorKey(highestLodKey, level);
	}

	return boost::none;
}

} // namespace vis
} // namespace skybolt