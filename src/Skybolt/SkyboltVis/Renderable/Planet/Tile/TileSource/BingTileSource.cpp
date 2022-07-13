/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BingTileSource.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/ShaUtility.h>

#include <httplib/httplib.h>
#include <osg/Vec2i>

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

static std::string getUrlTemplate(const std::string& str)
{
	size_t pos = str.find("\"imageUrl\":\"");
	std::string sub = str.substr(pos + 12);
	size_t end = sub.find_first_of('"');
	return sub.substr(0, end);
}

static std::string tileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
{
	std::string quadKey;
	for (int i = levelOfDetail; i > 0; i--)
	{
		char digit = '0';
		int mask = 1 << (i - 1);
		if ((tileX & mask) != 0)
		{
			digit++;
		}
		if ((tileY & mask) != 0)
		{
			digit++;
			digit++;
		}
		quadKey.push_back(digit);
	}
	return quadKey;
}

BingTileSource::BingTileSource(const BingTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mCacheSha(skybolt::calcSha1(config.url))
{

	httplib::Client cli(config.url.c_str());

	std::string request = "/REST/V1/Imagery/Metadata/Aerial?output=json&include=ImageryProviders&key=" + config.apiKey;
	auto res = cli.Get(request.c_str());
	if (res)
	{
		if (res->status == 200)
		{
			std::string templateString = getUrlTemplate(res->body);
			boost::replace_all(templateString, "\\", "");
			boost::replace_all(templateString, "{subdomain}", "t1");
			size_t pos = templateString.find("{quadkey}");
			mUrlPartBeforeTileKey = templateString.substr(0, pos);
			mUrlPartAfterTileKey = templateString.substr(pos + 9);
		}
		else
		{
			BOOST_LOG_TRIVIAL(error) << "BingTileSource received error response '" << res->status << "' from http request";
		}
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "BingTileSource timed out waiting for http response";
	}
}

osg::ref_ptr<osg::Image> BingTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	if (mUrlPartBeforeTileKey.empty())
	{
		return nullptr;
	}

	std::string url = mUrlPartBeforeTileKey + tileXYToQuadKey(key.x, key.y, key.level) + mUrlPartAfterTileKey;
	return readImageWithoutWarnings(url);
}

} // namespace vis
} // namespace skybolt
