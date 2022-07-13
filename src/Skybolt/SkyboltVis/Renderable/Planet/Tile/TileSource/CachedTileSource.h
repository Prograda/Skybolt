/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileSource.h"
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {
namespace vis {

class CachedTileSource : public TileSource
{
public:
	CachedTileSource(const TileSourcePtr& tileSource, const std::string& cacheDirectory);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

	bool hasAnyChildren(const skybolt::QuadTreeTileKey& key) const override
	{
		return mTileSource->hasAnyChildren(key);
	}

	std::optional<skybolt::QuadTreeTileKey> getHighestAvailableLevel(const skybolt::QuadTreeTileKey& key) const override
	{
		return mTileSource->getHighestAvailableLevel(key);
	}

	const std::string& getCacheSha() const override { throw std::runtime_error("Cached tile source cann't be cached"); }

private:
	TileSourcePtr mTileSource;
	std::string mCacheDirectory;
};

} // namespace vis
} // namespace skybolt
