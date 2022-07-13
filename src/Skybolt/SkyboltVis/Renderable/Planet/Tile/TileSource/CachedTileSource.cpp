/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CachedTileSource.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"

#include <osgDB/WriteFile>

#include <filesystem>

namespace skybolt {
namespace vis {

CachedTileSource::CachedTileSource(const TileSourcePtr& tileSource, const std::string& cacheDirectory) :
	mTileSource(tileSource),
	mCacheDirectory(cacheDirectory)
{
	assert(mTileSource);
}

osg::ref_ptr<osg::Image> CachedTileSource::createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	std::string imageDirectory = mCacheDirectory + "/" + std::to_string(key.level) + "/" + std::to_string(key.x) + "/";
	std::string filename = imageDirectory + std::to_string(key.y) + "." + mTileSource->getCacheFileFormat();

	const bool supportUserData = (mTileSource->getCacheFileFormat() == "pngx");

	if (std::filesystem::exists(filename))
	{
		osg::ref_ptr<osg::Image> image;
		if (supportUserData)
		{
			std::ifstream f(filename.c_str(), std::ios::binary);
			image = readImageWithUserData(f, "png");
			f.close();
		}
		else
		{
			image = readImageWithoutWarnings(filename);
		}
		if (isHeightMapDataFormat(*image))
		{
			image->setInternalTextureFormat(getHeightMapInternalTextureFormat());
		}
		return image;
	}
	else
	{
		osg::ref_ptr<osg::Image> image = mTileSource->createImage(key, cancelSupplier);
		if (image)
		{
			std::filesystem::create_directories(imageDirectory);

			if (supportUserData)
			{
				std::ofstream f(filename.c_str(), std::ios::binary);
				writeImageWithUserData(*image, f, "png");
				f.close();
			}
			else
			{
				osgDB::writeImageFile(*image, filename);
			}
		}
		return image;
	}
}

} // namespace vis
} // namespace skybolt
