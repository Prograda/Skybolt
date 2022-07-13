/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetTileImagesLoader.h"
#include "TileSource/TileSource.h"
#include "SkyboltVis/Renderable/Planet/AttributeMapHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h"
#include "SkyboltVis/Renderable/Planet/Tile/NormalMapHelpers.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include <algorithm>
#include <osg/Texture>

//#define ENABLE_TILE_IMAGE_LOADER_PROFILING
#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
#include <cxxtimer/cxxtimer.hpp>
#endif

using namespace skybolt;

namespace skybolt {
namespace vis {

static osg::Image* createDefaultHeightImage(const HeightMapElevationRerange& rerange)
{
	const int oceanHeight = getColorValueForElevation(rerange, 0.f);

	osg::Image* image = new osg::Image();
	image->allocateImage(256, 256, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	image->setInternalTextureFormat(GL_R16);
	uint16_t* ptr = (uint16_t*)image->getDataPointer();
	for (int i = 0; i < 256 * 256; ++i)
	{
		ptr[i] = oceanHeight;
	}

	setHeightMapElevationBounds(*image, {0,0});
	setHeightMapElevationRerange(*image, rerange);

	return image;
}

static osg::Image* createDefaultAlbedoImage()
{
	osg::Image* image = new osg::Image();
	image->allocateImage(256, 256, 1, GL_RGB, GL_BYTE);
	image->setInternalTextureFormat(GL_RGB8);
	memset((char*)(image->getDataPointer()), 0, 3 * 256 * 256);
	return image;
}

static osg::Image* convertHeightmapToLandMask(const osg::Image& src, const HeightMapElevationRerange& rerange)
{
	const int oceanHeight = getColorValueForElevation(rerange, 0.f);

	osg::Image* dst = new osg::Image;
	dst->allocateImage(src.s(), src.t(), 1, GL_ALPHA, GL_UNSIGNED_BYTE);
	dst->setInternalTextureFormat(GL_ALPHA8);

	uint16_t* pSrc = (uint16_t*)src.data();
	unsigned char* pDst = (unsigned char*)dst->data();
	size_t size = src.s() * src.t();
	for (size_t i = 0; i < size; ++i)
	{
		pDst[i] = (pSrc[i] <= oceanHeight) ? 0 : 255;
	}

	return dst;
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

	static HeightMapElevationRerange defaultRerange = {1, 0};
	static osg::ref_ptr<osg::Image> defaultHeightImage = createDefaultHeightImage(defaultRerange);
	static osg::ref_ptr<osg::Image> defaultNormalMap = createNormalMapFromHeightMap(*defaultHeightImage, defaultRerange, osg::Vec2(1,1));
	static osg::ref_ptr<osg::Image> defaultLandMask = convertHeightmapToLandMask(*defaultHeightImage, defaultRerange);

	// Height map
	{
		std::optional<QuadTreeTileKey> elevationKey = elevationLayer->getHighestAvailableLevel(key);
		if (elevationKey)
		{
			images->heightMapImage = getOrCreateImage(*elevationKey, size_t(CacheIndex::Elevation), [this, cancelSupplier](const QuadTreeTileKey& key) {
				return elevationLayer->createImage(key, cancelSupplier);
			});
		}

		if (images->heightMapImage.image)
		{
			osg::ref_ptr<osg::Image> heightImage = images->heightMapImage.image;
			auto bounds = getKeyLonLatBounds<osg::Vec2>(images->heightMapImage.key);
			osg::Vec2 heightImageLonLatDelta = bounds.size();
			osg::Vec2 texelWorldSize = osg::Vec2f(
				heightImageLonLatDelta.x() * mPlanetRadius * std::cos(bounds.center().y()) / heightImage->s(),
				heightImageLonLatDelta.y() * mPlanetRadius / heightImage->t()
			);
			int filterWidth = 5;
			images->normalMapImage = createNormalMapFromHeightMap(*heightImage, getRequiredHeightMapElevationRerange(*heightImage), texelWorldSize, filterWidth);
		}
		else
		{
			images->heightMapImage.image = defaultHeightImage;
			images->normalMapImage = defaultNormalMap;
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Height," << key.level << "," << timer.count() << std::endl;
		timer.reset();
		timer.start();
#endif
	}

	// Land mask
	{
		osg::ref_ptr<osg::Image> heightImage = images->heightMapImage.image;
		images->landMaskImage = getOrCreateImage(images->heightMapImage.key, size_t(CacheIndex::LandMask), [this, heightImage, cancelSupplier](const QuadTreeTileKey& key) {
			if (landMaskLayer)
			{
				return landMaskLayer->createImage(key, cancelSupplier);
			}
			else
			{
				if (heightImage == defaultHeightImage)
				{
					return defaultLandMask;
				}
				osg::ref_ptr<osg::Image> image = convertHeightmapToLandMask(*heightImage, getRequiredHeightMapElevationRerange(*heightImage));
				return image;
			}
		}).image;

		if (!images->landMaskImage)
		{
			images->landMaskImage = defaultLandMask;
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Land," << key.level << "," << timer.count() << std::endl;
		timer.reset();
		timer.start();
#endif
	}

	// Albedo map
	{
		static osg::ref_ptr<osg::Image> defaultAlbedoImage = createDefaultAlbedoImage();

		std::optional<QuadTreeTileKey> albedoKey = albedoLayer->getHighestAvailableLevel(key);
		if (albedoKey)
		{
			images->albedoMapImage = getOrCreateImage(*albedoKey, size_t(CacheIndex::Albedo), [this, cancelSupplier](const QuadTreeTileKey& key) {
				osg::ref_ptr<osg::Image> image = albedoLayer->createImage(key, cancelSupplier);
				return image;
			});
		}

		if (!images->albedoMapImage.image)
		{
			images->albedoMapImage.image = defaultAlbedoImage;
		}

	#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		std::cout << "Albedo," << key.level << "," << timer.count() << std::endl;
		timer.reset();
		timer.start();
	#endif
	}

	// Attribute map
	{
		if (attributeLayer)
		{
			std::optional<QuadTreeTileKey> attributeKey = attributeLayer->getHighestAvailableLevel(key);

			images->attributeMapImage = getOrCreateImage(*attributeKey, size_t(CacheIndex::Attribute), [this, cancelSupplier](const QuadTreeTileKey& key) {
				osg::ref_ptr<osg::Image> image = attributeLayer->createImage(key, cancelSupplier);
				if (image)
				{
					image = convertAttributeMap(*image, getNlcdAttributeColors());
				}
				return image;
			});
			if (!images->attributeMapImage->image)
			{
				images->attributeMapImage = std::nullopt;
			}
		}
		else if (!images->attributeMapImage && false) // Experimental. If enabled, attribute map will be generated from the albedo map, otherwise no attributes will be used.
		{
			images->attributeMapImage = getOrCreateImage(key, size_t(CacheIndex::Attribute), [this, cancelSupplier, albedo = images->albedoMapImage.image](const QuadTreeTileKey& key) {
				return convertToAttributeMap(*albedo);
			});
		}

#ifdef ENABLE_TILE_IMAGE_LOADER_PROFILING
		printf("Attribute, %i, %i\n", key.level, timer.count());
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
