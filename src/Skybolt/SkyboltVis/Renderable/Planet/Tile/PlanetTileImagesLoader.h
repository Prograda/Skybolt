/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileImagesLoader.h"
#include "SkyboltVis/SkyboltVisFwd.h"

namespace skybolt {
namespace vis {

struct PlanetTileImages : TileImages
{
	~PlanetTileImages() override = default;

	TileImage heightMapImage;
	ImagePtr normalMapImage; //!< Same tile key as heightMapImage
	ImagePtr landMaskImage; //!< Same tile key as heightMapImage

	TileImage albedoMapImage;

	//! Item indices correspond to PlanetTileImagesLoader::attributeLayers. Item will be nullopt if attribute could not be loaded.
	std::vector<std::optional<TileImage>> attributeMapImages;
};

enum AttributeMapProcessing
{
	None,
	ConvertNlcdAttributeColors
};

class PlanetTileImagesLoader : public TileImagesLoader
{
public:
	struct Layers
	{
		TileSourcePtr elevation; //!< never null
		TileSourcePtr landMask; //!< if null and generateLandMaskFromElevation = true, land mask is auto generated from elevation, otherwise attributes are not used
		TileSourcePtr albedo; //!< never null

		struct AttributeLayer
		{
			TileSourcePtr source; //!< never null
			AttributeMapProcessing processing = AttributeMapProcessing::None;
		};

		std::vector<AttributeLayer> attributes; //!< if empty, attributes are not used. Items must be non-null.
	};
	const Layers layers;

	//! If true, land mask is generated from elevation if landMaskLayer is null.
	bool generateLandMaskFromElevation = true;

	PlanetTileImagesLoader(const ImageFactoryPtr& imageFactory, double planetRadius, Layers layers);
	//! May be called from multiple threads
	TileImagesPtr load(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

private:
	ImageFactoryPtr mImageFactory;
	const double mPlanetRadius;
};

} // namespace vis
} // namespace skybolt
