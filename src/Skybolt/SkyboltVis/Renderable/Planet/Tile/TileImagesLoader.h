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

struct TileImages
{
	virtual ~TileImages() = default;
};

using TileImagesPtr = std::shared_ptr<TileImages>;

class TileImagesLoader
{
public:
	TileImagesLoader(size_t imageCount) :
		mImageCache(imageCount),
		mCacheMutex(imageCount)
	{
	}

	virtual ~TileImagesLoader() = default;

	//! Loads a set of images for the tile at the given key.
	//! For example, images in the set might include a height map, albedo map, normal map etc.
	//! May be called from multiple threads.
	//! Returns nullptr on cancel.
	virtual TileImagesPtr load(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const = 0;

protected:
	struct CacheEntry
	{
		TileImage image;
		std::mutex mImageMutex;
	};

	typedef std::shared_ptr<CacheEntry> CacheEntryPtr;
	typedef std::map<skybolt::QuadTreeTileKey, CacheEntryPtr> TileCache; //!< Maps a requested tile key to an image. The image may be at a lower key level than the request e.g if no high res image is available.
	typedef std::function<osg::ref_ptr<osg::Image>(const skybolt::QuadTreeTileKey& key)> Factory;

	//! @param minLevel is the minimum level of the image allowed to be returned as a fallback if the requested key level is not found.
	//! If no image was found at an allowed level, nullptr is returned.
	TileImage getOrCreateImage(const skybolt::QuadTreeTileKey& requestedKey, size_t cacheIndex, Factory factory, int minLevel = 0) const;


private:
	mutable std::vector<TileCache> mImageCache;
	mutable std::vector<std::mutex> mCacheMutex;
};

} // namespace vis
} // namespace skybolt
