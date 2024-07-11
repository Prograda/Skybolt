/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "TileSource.h"
#include "SkyboltVis/SkyboltVisFwd.h"

namespace skybolt {
namespace vis {

//! A Plate Carree projection TileSource that wraps a Spherical Mercator projection TileSource
class SphericalMercatorToPlateCarreeTileSource : public TileSource
{
public:
	SphericalMercatorToPlateCarreeTileSource(const TileSourcePtr& source);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

	bool hasAnyChildren(const skybolt::QuadTreeTileKey& key) const override
	{
		return mTileSource->hasAnyChildren(key);
	}

	std::optional<skybolt::QuadTreeTileKey> getHighestAvailableLevel(const skybolt::QuadTreeTileKey& key) const override
	{
		return mTileSource->getHighestAvailableLevel(key);
	}

	const std::string& getCacheSha() const override { return mTileSource->getCacheSha(); }
	const std::string& getCacheFileFormat() const override { return mTileSource->getCacheFileFormat(); }

private:
	TileSourcePtr mTileSource;
};

} // namespace vis
} // namespace skybolt
