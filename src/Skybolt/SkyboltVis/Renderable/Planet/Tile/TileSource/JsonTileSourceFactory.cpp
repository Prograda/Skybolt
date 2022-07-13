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
#include <SkyboltCommon/Json/JsonHelpers.h>

namespace skybolt {
namespace vis {

JsonTileSourceFactoryRegistry::JsonTileSourceFactoryRegistry(const JsonTileSourceFactoryRegistryConfig& config) :
	mCacheDirectory(config.cacheDirectory),
	mApiKeys(config.apiKeys)
{
}

void JsonTileSourceFactoryRegistry::addFactory(const std::string& name, JsonTileSourceFactory factory)
{
	mFactories[name] = std::move(factory);
}

const JsonTileSourceFactory& JsonTileSourceFactoryRegistry::getFactory(const std::string& name) const
{
	auto i = mFactories.find(name);
	if (i == mFactories.end())
	{
		throw std::runtime_error("Unsupported tile source format: " + name);
	}

	return i->second;
}

JsonTileSourceFactory JsonTileSourceFactoryRegistry::wrapWithCacheSupport(JsonTileSourceFactory factory) const
{
	return [factory = std::move(factory), this] (const nlohmann::json& json) -> TileSourcePtr {
		auto tileSource = factory(json);
		auto i = json.find("cache");
		if (i != json.end())
		{
			if (i->get<bool>())
			{
				std::string url = json.at("url");
				std::string directory = mCacheDirectory + "/" + tileSource->getCacheSha();
				return std::make_shared<CachedTileSource>(tileSource, directory);
			}
		}
		return tileSource;
	};
}

JsonTileSourceFactory JsonTileSourceFactoryRegistry::wrapWithProjectionSupport(JsonTileSourceFactory factory) const
{
	return[factory = std::move(factory), this] (const nlohmann::json& json) -> TileSourcePtr {
		auto tileSource = factory(json);
		if (json.at("projection") == "sphericalMercator")
		{
			return std::make_shared<SphericalMercatorToPlateCarreeTileSource>(tileSource);
		}
		return tileSource;
	};
}

const std::string& getApiKey(const ApiKeys& keys, const std::string& name)
{
	auto i = keys.find(name);
	if (i == keys.end())
	{
		throw std::runtime_error("Unknown tile API key: " + name);
	}
	return i->second;
}

static JsonTileSourceFactory wrapAll(const JsonTileSourceFactoryRegistry& registry, JsonTileSourceFactory factory)
{
	return registry.wrapWithCacheSupport(
		registry.wrapWithProjectionSupport(factory));
}

static IntRangeInclusive readLevelRange(const nlohmann::json& json)
{
	int minLevel = readOptionalOrDefault(json, "minLevel", 0);
	int maxLevel = json.at("maxLevel");
	return IntRangeInclusive(minLevel, maxLevel);
}

void addDefaultFactories(JsonTileSourceFactoryRegistry& registry)
{
	ApiKeys keys = registry.getApiKeys();
	registry.addFactory("xyz", wrapAll(registry, [keys] (const nlohmann::json& json) {
		std::string apiKey;
		auto i = json.find("apiKeyName");
		if (i != json.end())
		{
			apiKey = getApiKey(keys, i.value());
		}

		XyzTileSourceConfig xyzConfig;
		xyzConfig.urlTemplate = json.at("url");
		xyzConfig.yOrigin = readOptionalOrDefault(json, "yTileOriginAtBottom", false) ? XyzTileSourceConfig::YOrigin::Bottom : XyzTileSourceConfig::YOrigin::Top;
		xyzConfig.apiKey = apiKey;
		xyzConfig.levelRange = readLevelRange(json);
		xyzConfig.imageType = readOptionalOrDefault(json, "produceElevation", false) ? XyzTileSourceConfig::ImageType::Elevation : XyzTileSourceConfig::ImageType::Color;
		auto source = std::make_shared<XyzTileSource>(xyzConfig);
		source->validate();
		return source;
	}));

	registry.addFactory("bing", wrapAll(registry, [keys] (const nlohmann::json& json) {
		BingTileSourceConfig bingConfig;
		bingConfig.url = json.at("url");
		bingConfig.apiKey = getApiKey(keys, "bing");
		bingConfig.levelRange = readLevelRange(json);
		return std::make_shared<BingTileSource>(bingConfig);
	}));

	registry.addFactory("mapboxElevation", wrapAll(registry, [keys] (const nlohmann::json& json) {
		MapboxElevationTileSourceConfig config;
		config.urlTemplate = json.at("url");
		config.apiKey = getApiKey(keys, "mapbox");
		config.levelRange = readLevelRange(json);
		return std::make_shared<MapboxElevationTileSource>(config);
	}));
}

} // namespace vis
} // namespace skybolt
