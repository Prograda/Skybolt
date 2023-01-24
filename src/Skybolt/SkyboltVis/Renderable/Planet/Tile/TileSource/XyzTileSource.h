/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "TileSourceWithMinMaxLevel.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"

namespace skybolt {
namespace vis {

struct XyzTileSourceConfig
{
	//! URL must be templated with variables x, y, z, key in curley braces
	//! E.g "https://test.com/image/{z}/{x}/{y}.png?{key}"
	std::string urlTemplate;

	std::string apiKey;

	enum class YOrigin
	{
		Top, //!< y=0 tile is at the top of the image
		Bottom //!< y=0 tile is at the bottom of the image
	};

	YOrigin yOrigin = YOrigin::Top;

	IntRangeInclusive levelRange;
	std::optional<HeightMapElevationRerange> elevationRerange; //!< If provided, treat images as heightmaps storing elevation with the given rerange
};

class XyzTileSource : public TileSourceWithMinMaxLevel
{
public:
	XyzTileSource(const XyzTileSourceConfig& config);

	//! @return true if images can be loaded from URL
	bool validate() const;

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

	const std::string& getCacheSha() const override { return mCacheSha; }

	const std::string& getCacheFileFormat() const override
	{
		static const std::string pngStr = "png";
		static const std::string pngxStr = "pngx";
		return mElevationRerange ? pngxStr : pngStr;
	}

private:
	std::string toUrl(const skybolt::QuadTreeTileKey& key) const;

private:
	const std::string mUrlTemplate;
	const XyzTileSourceConfig::YOrigin mYOrigin;
	const std::string mApiKey;
	const std::string mCacheSha;
	std::optional<HeightMapElevationRerange> mElevationRerange;
};

} // namespace vis
} // namespace skybolt
