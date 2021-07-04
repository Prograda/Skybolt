/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileImagesLoader.h"

namespace skybolt {
namespace vis {

TileImage TileImagesLoader::getOrCreateImage(const QuadTreeTileKey& requestedKey, size_t cacheIndex, Factory factory, int minLevel) const
{
	TileCache& cache = mImageCache[cacheIndex];

	bool added = false;
	CacheEntryPtr entry;
	{
		{
			std::lock_guard<std::mutex> lock(mCacheMutex[cacheIndex]);
			entry = cache[requestedKey];
		}
		if (!entry)
		{
			entry = std::make_shared<CacheEntry>();
			added = true;
		}
	}

	if (added)
	{
		std::lock_guard<std::mutex> lock(entry->mImageMutex);

		int level = requestedKey.level;
		QuadTreeTileKey key = requestedKey;
		while (level >= minLevel)
		{
			entry->image.image = factory(key);
			if (entry->image.image)
			{
				entry->image.key = key;
				break;
			}
			--level;
			key = createAncestorKey(requestedKey, level);
		}
	}
	return entry->image;
}

} // namespace vis
} // namespace skybolt
