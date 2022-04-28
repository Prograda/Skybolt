/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "XyzTileSource.h"

#include "SkyboltVis/OsgImageHelpers.h"

#include <boost/algorithm/string/replace.hpp>

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

static int flipY(int y, int level)
{
	return (1 << level) - y - 1;
}

osg::ref_ptr<osg::Image> XyzTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	int y = (mYOrigin == XyzTileSourceConfig::YOrigin::Top) ? key.y : flipY(key.y, key.level);

	std::string url = mUrlTemplate;
	boost::replace_all(url, "{x}", std::to_string(key.x));
	boost::replace_all(url, "{y}", std::to_string(y));
	boost::replace_all(url, "{z}", std::to_string(key.level));
	boost::replace_all(url, "{key}", mApiKey);

	osg::ref_ptr<osg::Image> image = readImageWithoutWarnings(url);
	return image;
}

} // namespace vis
} // namespace skybolt
