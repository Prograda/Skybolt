/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileImage.h"
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {
namespace vis {

struct LeafTileData
{
	TileImage heightMapImage;
	osg::ref_ptr<osg::Image> landMaskImage; //!< Same tile key as heightMapImage

	TileImage albedoMapImage;
	TileImage attributeMapImage; //!< Optional
};

typedef std::shared_ptr<LeafTileData> LeafTileDataPtr;

class TileImageLoader
{
public:
	TileSourcePtr elevationLayer;
	TileSourcePtr landUseLayer; //!< can be null
	TileSourcePtr albedoLayer;
	int maxElevationLod;
	int minAttributeLod; //!< Load attribute tiles for lod levels of at least this

	enum class CacheIndex
	{
		Elevation,
		LandMask,
		Albedo,
		LandUse
	};

	//! May be called from multiple threads
	LeafTileDataPtr load(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const;

private:
	struct CacheEntry
	{
		TileImage image;
		std::mutex mImageMutex;
	};

	typedef std::shared_ptr<CacheEntry> CacheEntryPtr;
	typedef std::map<skybolt::QuadTreeTileKey, CacheEntryPtr> TileCache; //!< Maps a requested tile key to an image. The image may be at a lower key level than the request e.g if no high res image is available.
	typedef std::function<osg::ref_ptr<osg::Image>(const skybolt::QuadTreeTileKey& key)> Factory;

	TileImage getOrCreateImage(const skybolt::QuadTreeTileKey& requestedKey, CacheIndex cacheIndex, Factory factory) const;


private:
	mutable TileCache mImageCache[4];
	mutable std::mutex mCacheMutex[4];
};

} // namespace vis
} // namespace skybolt
