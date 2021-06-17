/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileImagesLoader.h"

namespace skybolt {
namespace vis {

struct PlanetTileImages : TileImages
{
	~PlanetTileImages() override = default;

	TileImage heightMapImage;
	osg::ref_ptr<osg::Image> landMaskImage; //!< Same tile key as heightMapImage

	TileImage albedoMapImage;
	TileImage attributeMapImage; //!< Optional
};

class PlanetTileImagesLoader : public TileImagesLoader
{
public:
	TileSourcePtr elevationLayer; //!< never null
	TileSourcePtr landUseLayer; //!< can be null
	TileSourcePtr albedoLayer; //!< never null
	int maxElevationLod;
	int minAttributeLod; //!< Load attribute tiles for lod levels of at least this

	enum class CacheIndex
	{
		Elevation,
		LandMask,
		Albedo,
		LandUse
	};

	PlanetTileImagesLoader() : TileImagesLoader(4) {}

	//! May be called from multiple threads
	TileImagesPtr load(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;
};

} // namespace vis
} // namespace skybolt
