/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Image>

#include <atomic>
#include <string>

namespace skybolt {
namespace vis {

class TileSource
{
public:
	virtual ~TileSource() {}

	//! May be called concurrently from different threads to create different images.
	//! Images representing height maps are expected to have HeightMapElevationBounds and HeightMapElevationRerange user data.
	//!@ThreadSafe
	virtual osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const = 0;

	//! @returns true if tile source data exists for the children of the tile with the given key
	virtual bool hasAnyChildren(const skybolt::QuadTreeTileKey& key) const = 0;

	//! @returns the highest key with source data in the given key's ancestral hierarchy
	virtual std::optional<skybolt::QuadTreeTileKey> getHighestAvailableLevel(const skybolt::QuadTreeTileKey& key) const = 0;

	//! @return unique sha of parameters affecting the visual appearance of tiles, e.g URL. Used to determine name of this TileSource's cached tiles folder.
	virtual const std::string& getCacheSha() const = 0;

	virtual const std::string& getCacheFileFormat() const
	{
		static const std::string s = "png";
		return s;
	}
};

} // namespace vis
} // namespace skybolt
