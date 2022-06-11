/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "XyzTileSource.h"

#include "SkyboltVis/OsgImageHelpers.h"
#include <osgDB/ReadFile>
#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

XyzTileSource::XyzTileSource(const XyzTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mUrlTemplate(config.urlTemplate),
	mYOrigin(config.yOrigin),
	mApiKey(config.apiKey)
{
}

bool XyzTileSource::validate() const
{
	// Validate the loader by loading level 0 image
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(toUrl(QuadTreeTileKey()));
	if (!image)
	{
		BOOST_LOG_TRIVIAL(error) << "Could not load image from XyzTileSource with URL template '" << mUrlTemplate << ".";
	}
	return image != nullptr;
}

static int flipY(int y, int level)
{
	return (1 << level) - y - 1;
}

osg::ref_ptr<osg::Image> XyzTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	return readImageWithoutWarnings(toUrl(key));
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
