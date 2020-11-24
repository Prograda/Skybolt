/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "JsonTileSourceFactory.h"

#include "SkyboltVis/Renderable/Planet/Tile/TileSource/BingTileSource.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/CachedTileSource.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/MapboxElevationTileSource.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/SphericalMercatorToPlateCarreeTileSource.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h"
#include <SkyboltCommon/ShaUtility.h>
#include <SkyboltCommon/Json/JsonHelpers.h>

namespace skybolt {
namespace vis {

JsonTileSourceFactory::JsonTileSourceFactory(const JsonTileSourceFactoryConfig& config) :
	mCacheDirectory(config.cacheDirectory),
	mApiKeys(config.apiKeys)
{
}

TileSourcePtr JsonTileSourceFactory::createTileSourceFromJson(const nlohmann::json& json) const
{
	TileSourcePtr tileSource;

	std::string format = json.at("format");
	std::string url = json.at("url");

	if (format == "xyz")
	{
		std::string apiKey;
		auto i = json.find("apiKeyName");
		if (i != json.end())
		{
			apiKey = getApiKey(i.value());
		}

		XyzTileSourceConfig xyzConfig;
		xyzConfig.urlTemplate = url;
		xyzConfig.yOrigin = readOptionalOrDefault(json, "yTileOriginAtBottom", false) ? XyzTileSourceConfig::YOrigin::Bottom : XyzTileSourceConfig::YOrigin::Top;
		xyzConfig.apiKey = apiKey;
		tileSource = std::make_shared<XyzTileSource>(xyzConfig);
	}
	else if (format == "bing")
	{
		BingTileSourceConfig bingConfig;
		bingConfig.url = url;
		bingConfig.apiKey = getApiKey("bing");
		tileSource = std::make_shared<BingTileSource>(bingConfig);
	}
	else if (format == "mapboxElevation")
	{
		MapboxElevationTileSourceConfig config;
		config.urlTemplate = url;
		config.apiKey = getApiKey("mapbox");
		tileSource = std::make_shared<MapboxElevationTileSource>(config);
	}
	else
	{
		throw std::runtime_error("Unsupported tile source format: " + format);
	}

	if (json.at("projection") == "sphericalMercator")
	{
		tileSource = std::make_shared<SphericalMercatorToPlateCarreeTileSource>(tileSource);
	}

	auto it = json.find("cache");
	if (it != json.end())
	{
		if (it->get<bool>())
		{
			std::string directory = mCacheDirectory + "/" + calcSha1(url);
			tileSource = std::make_shared<CachedTileSource>(tileSource, directory);
		}
	}
	return tileSource;
}

std::string JsonTileSourceFactory::getApiKey(const std::string& name) const
{
	auto i = mApiKeys.find(name);
	if (i == mApiKeys.end())
	{
		throw std::runtime_error("Unknown tile API key: " + name);
	}
	return i->second;
}

} // namespace vis
} // namespace skybolt
