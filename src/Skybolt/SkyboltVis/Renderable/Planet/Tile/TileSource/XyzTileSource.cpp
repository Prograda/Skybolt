/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "XyzTileSource.h"

#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"
#include <osgDB/ReadFile>
#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <SkyboltCommon/ShaUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

XyzTileSource::XyzTileSource(const XyzTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mUrlTemplate(config.urlTemplate),
	mYOrigin(config.yOrigin),
	mApiKey(config.apiKey),
	mCacheSha(skybolt::calcSha1(config.urlTemplate)),
	mImageType(config.imageType)
{
}

bool XyzTileSource::validate() const
{
	// Validate the loader by loading level 0 image
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(toUrl(QuadTreeTileKey()));
	if (!image)
	{
		BOOST_LOG_TRIVIAL(error) << "Could not load image from XyzTileSource with URL template '" << mUrlTemplate << ".";
		return false;
	}

	if (mImageType == XyzTileSourceConfig::ImageType::Elevation)
	{
		if (image->getPixelFormat() != GL_LUMINANCE)
		{
			BOOST_LOG_TRIVIAL(error) << "Elevation image with URL template '" << mUrlTemplate << "' is in wrong format: "
				<< image->getPixelFormat() << ". It should be GL_LUMINANCE.'";
		}
		if (image->getDataType() != GL_UNSIGNED_SHORT)
		{
			BOOST_LOG_TRIVIAL(error) << "Elevation image with URL template '" << mUrlTemplate << "' is in wrong format: "
				<< image->getPixelFormat() << ". It should be GL_UNSIGNED_SHORT.'";
		}
		return false;
	}

	return true;
}

static int flipY(int y, int level)
{
	return (1 << level) - y - 1;
}

osg::ref_ptr<osg::Image> XyzTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	osg::ref_ptr<osg::Image> image = readImageWithoutWarnings(toUrl(key));

	if (mImageType == XyzTileSourceConfig::ImageType::Elevation)
	{
		if (!isHeightMapDataFormat(*image))
		{
			return nullptr;
		}
		image->setInternalTextureFormat(getHeightMapInternalTextureFormat());

		const HeightMapElevationRerange& rerange = getDefaultEarthRerange();
		setHeightMapElevationRerange(*image, rerange);

		HeightMapElevationBounds bounds = emptyHeightMapElevationBounds();
		uint16_t* p = reinterpret_cast<uint16_t*>(image->data());
		int elementCount = image->s() * image->t();
		for (int i = 0; i < elementCount; ++i)
		{
			expand(bounds, getElevationForColorValue(rerange, *p));
		}
		setHeightMapElevationBounds(*image, bounds);

	}

	return image;
}

std::string XyzTileSource::toUrl(const skybolt::QuadTreeTileKey& key) const
{
	int y = (mYOrigin == XyzTileSourceConfig::YOrigin::Top) ? key.y : flipY(key.y, key.level);

	std::string url = mUrlTemplate;
	boost::replace_all(url, "{x}", std::to_string(key.x));
	boost::replace_all(url, "{y}", std::to_string(y));
	boost::replace_all(url, "{z}", std::to_string(key.level));
	boost::replace_all(url, "{key}", mApiKey);
	return url;
}

} // namespace vis
} // namespace skybolt
