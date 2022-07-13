/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "XyzTileSource.h"

namespace skybolt {
namespace vis {

struct MapboxElevationTileSourceConfig
{
	//! URL must be templated with variables x, y, z, key in curley braces
	//! E.g http://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token={key}
	std::string urlTemplate;
	std::string apiKey;
	IntRangeInclusive levelRange;
};

class MapboxElevationTileSource : public TileSourceWithMinMaxLevel
{
public:
	MapboxElevationTileSource(const MapboxElevationTileSourceConfig& config);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

	const std::string& getCacheSha() const override { return mCacheSha; }

	const std::string& getCacheFileFormat() const override
	{
		static const std::string s = "pngx";
		return s;
	}

private:
	const std::string mCacheSha;
	std::unique_ptr<XyzTileSource> mSource;
};

} // namespace vis
} // namespace skybolt
