/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetTileImagesLoader.h"
#include "TileSource/TileSource.h"
#include "SkyboltVis/Elevation/ElevationImageMetadata.h"
#include "SkyboltVis/Image/ImageFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/AttributeMapHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/NormalMapHelpers.h"
#include "SkyboltVis/Elevation/ElevationBounds.h"
#include <algorithm>

//#define ENABLE_TILE_IMAGE_LOADER_PROFILING
#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
#include <cxxtimer/cxxtimer.hpp>
#include <iostream>
#endif

using namespace skybolt;

namespace skybolt {
namespace vis {

static ImagePtr createDefaultHeightImage(const ImageFactory& imageFactory, const ElevationRerange& rerange)
{
	const int oceanHeight = getColorValueForElevation(rerange, 0.f);

	ImagePtr image = valueOrThrowException(imageFactory.createImage(256, 256, Image::Format::R16, Image::ColorSpace::Linear));
	uint16_t* ptr = (uint16_t*)image->getRawData();
	for (int i = 0; i < 256 * 256; ++i)
	{
		ptr[i] = oceanHeight;
	}

	ElevationImageMetadata metadata;
	metadata.elevationBounds = {0,0};
	metadata.rerange = rerange;
	setElevationImageMetadata(*image, metadata);

	return image;
}

static ImagePtr createDefaultAlbedoImage(const ImageFactory& imageFactory)
{
	ImagePtr image = valueOrThrowException(imageFactory.createImage(256, 256, Image::Format::RGB8, Image::ColorSpace::Srgb));
	memset((char*)(image->getRawData()), 0, 3 * 256 * 256);
	return image;
}

static ImagePtr convertHeightmapToLandMask(const ImageFactory& imageFactory, const Image& src, const ElevationRerange& rerange)
{
	const int oceanHeight = getColorValueForElevation(rerange, 0.f);

	ImagePtr dst = valueOrThrowException(imageFactory.createImage(src.getWidth(), src.getHeight(), Image::Format::R8, Image::ColorSpace::Linear));

	uint16_t* pSrc = (uint16_t*)src.getRawData();
	unsigned char* pDst = dst->getRawData();
	size_t size = src.getWidth() * src.getHeight();
	for (size_t i = 0; i < size; ++i)
	{
		pDst[i] = (pSrc[i] <= oceanHeight) ? 0 : 255;
	}

	return dst;
}

enum class CacheIndex
{
	Elevation,
	LandMask,
	Albedo,
	Attribute0,
	// Attribute1...AttributeN
};

PlanetTileImagesLoader::PlanetTileImagesLoader(const ImageFactoryPtr& imageFactory, double planetRadius, Layers layers) :
	TileImagesLoader(3 + layers.attributes.size()), // Count should be total number of CacheIndex values available
	mImageFactory(imageFactory),
	mPlanetRadius(planetRadius),
	layers(std::move(layers))
{
}

//! May be called from multiple threads
TileImagesPtr PlanetTileImagesLoader::load(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	if (cancelSupplier())
	{
		return nullptr;
	}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
	cxxtimer::Timer timer;
	timer.start();
#endif
	auto images = std::make_shared<PlanetTileImages>();

	static ElevationRerange defaultRerange = {1, 0};
	static ImagePtr defaultHeightImage = createDefaultHeightImage(*mImageFactory, defaultRerange);
	static ImagePtr defaultNormalMap = createNormalMapFromHeightMap(*mImageFactory, *defaultHeightImage, defaultRerange, glm::vec2(1,1));
	static ImagePtr defaultLandMask = convertHeightmapToLandMask(*mImageFactory, *defaultHeightImage, defaultRerange);

	// Height map
	{
		std::optional<QuadTreeTileKey> elevationKey = layers.elevation->getHighestAvailableLevel(key);
		if (elevationKey)
		{
			images->heightMapImage = getOrCreateImage(*elevationKey, size_t(CacheIndex::Elevation), [this, cancelSupplier](const QuadTreeTileKey& key) {
				ImagePtr image = layers.elevation->createImage(key, cancelSupplier);
				if (image)
				{
					image->setColorSpace(Image::ColorSpace::Linear);
				}
				return image;
			});
		}

		if (images->heightMapImage.image)
		{
			ImagePtr heightImage = images->heightMapImage.image;
			auto bounds = getKeyLonLatBounds<glm::vec2>(images->heightMapImage.key);
			glm::vec2 heightImageLonLatDelta = bounds.size();
			double latitude = std::cos(bounds.center().y);
			glm::vec2 texelWorldSize = glm::vec2(
				heightImageLonLatDelta.x * mPlanetRadius * latitude / heightImage->getWidth(), // East
				heightImageLonLatDelta.y * mPlanetRadius / heightImage->getHeight() // North
			);
			int filterWidth = 5;
			auto metadata = getElevationImageMetadataRequired(*heightImage);
			images->normalMapImage = createNormalMapFromHeightMap(*mImageFactory, *heightImage, metadata.rerange, texelWorldSize, filterWidth);
		}
		else
		{
			images->heightMapImage.image = defaultHeightImage;
			images->normalMapImage = defaultNormalMap;
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Height@" << key.level << ": " << timer.count() << std::endl;
		timer.reset();
		timer.start();
#endif
	}

	// Land mask
	{
		ImagePtr heightImage = images->heightMapImage.image;
		images->landMaskImage = getOrCreateImage(images->heightMapImage.key, size_t(CacheIndex::LandMask), [this, heightImage, cancelSupplier](const QuadTreeTileKey& key) {
			if (layers.landMask)
			{
				ImagePtr image = layers.landMask->createImage(key, cancelSupplier);
				if (image)
				{
					image->setColorSpace(Image::ColorSpace::Linear);
				}
				return image;
			}
			else
			{
				if (heightImage == defaultHeightImage)
				{
					return defaultLandMask;
				}
				auto metadata = getElevationImageMetadataRequired(*heightImage);
				ImagePtr image = convertHeightmapToLandMask(*mImageFactory, *heightImage, metadata.rerange);
				return image;
			}
		}).image;

		if (!images->landMaskImage)
		{
			images->landMaskImage = defaultLandMask;
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Land@" << key.level << ": " << timer.count() << std::endl;
		timer.reset();
		timer.start();
#endif
	}

	// Albedo map
	{
		static ImagePtr defaultAlbedoImage = createDefaultAlbedoImage(*mImageFactory);

		std::optional<QuadTreeTileKey> albedoKey = layers.albedo->getHighestAvailableLevel(key);
		if (albedoKey)
		{
			images->albedoMapImage = getOrCreateImage(*albedoKey, size_t(CacheIndex::Albedo), [this, cancelSupplier](const QuadTreeTileKey& key) {
				return layers.albedo->createImage(key, cancelSupplier);
			});
		}

		if (!images->albedoMapImage.image)
		{
			images->albedoMapImage.image = defaultAlbedoImage;
		}

	#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Albedo@" << key.level << ": " << timer.count() << std::endl;
		timer.reset();
		timer.start();
	#endif
	}

	// Attribute map
	{
		std::size_t attributeIndex = 0;
		images->attributeMapImages.resize(layers.attributes.size());
		for (const auto& attributeLayer : layers.attributes)
		{
			assert(attributeLayer.source);

			std::optional<QuadTreeTileKey> attributeKey = attributeLayer.source->getHighestAvailableLevel(key);
			if (attributeKey)
			{
				TileImage tileImage = getOrCreateImage(*attributeKey, size_t(CacheIndex::Attribute0) + attributeIndex, [this, attributeLayer, cancelSupplier](const QuadTreeTileKey& key) {
					ImagePtr image = attributeLayer.source->createImage(key, cancelSupplier);
					if (!image)
					{
						return image;
					}
					image->setColorSpace(Image::ColorSpace::Linear);
					if (attributeLayer.processing == AttributeMapProcessing::ConvertNlcdAttributeColors)
					{
						image = convertAttributeMap(*mImageFactory, *image, getNlcdAttributeColors());
					}
					return image;
				});
				if (tileImage.image)
				{
					images->attributeMapImages[attributeIndex] = std::move(tileImage);
				}
			}
			++attributeIndex;
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Attributes@" << key.level << ": " << timer.count();
		timer.reset();
		timer.start();
#endif
	}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
	std::cout << "Total TileImage load time: " << timer.count() << std::endl;
#endif
	if (cancelSupplier())
	{
		return nullptr;
	}
	return images;
}

} // namespace vis
} // namespace skybolt
