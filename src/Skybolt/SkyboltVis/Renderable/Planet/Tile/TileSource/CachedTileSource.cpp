/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CachedTileSource.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapHelpers.h"
#include "Image/Image.h"
#include "Image/ImageFactory.h"

#include <filesystem>

namespace skybolt {
namespace vis {

CachedTileSource::CachedTileSource(const ImageFactoryPtr& imageFactory, const TileSourcePtr& tileSource, const std::string& cacheDirectory) :
	mImageFactory(imageFactory),
	mTileSource(tileSource),
	mCacheDirectory(cacheDirectory)
{
	assert(mImageFactory);
	assert(mTileSource);
}

ImagePtr CachedTileSource::createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	std::string imageDirectory = mCacheDirectory + "/" + std::to_string(key.level) + "/" + std::to_string(key.x) + "/";
	std::string filename = imageDirectory + std::to_string(key.y) + "." + mTileSource->getCacheFileFormat();

	if (std::filesystem::exists(filename))
	{
		auto image = value(mImageFactory->readImage(filename)).value_or(nullptr);
		if (image && isHeightMapDataFormat(*image))
		{
			image->setColorSpace(Image::ColorSpace::Linear);
		}
		return image;
	}
	else
	{
		ImagePtr image = mTileSource->createImage(key, cancelSupplier);
		if (image)
		{
			std::filesystem::create_directories(imageDirectory);

			auto result = mImageFactory->writeImage(*image, filename);
			if (!has_value(result))
			{
				throw std::runtime_error("Could not write cached tile image to: " + filename + ". Reason: " + std::get<UnexpectedMessage>(result).str);
			}
		}
		return image;
	}
}

} // namespace vis
} // namespace skybolt
