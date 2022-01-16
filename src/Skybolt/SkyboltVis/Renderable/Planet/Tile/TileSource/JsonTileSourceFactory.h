/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"

#include <nlohmann/json.hpp>
#include <boost/optional.hpp>
#include <string>

namespace skybolt {
namespace vis {

struct JsonTileSourceFactoryRegistryConfig
{
	std::string cacheDirectory;
	std::map<std::string, std::string> apiKeys;
};

using JsonTileSourceFactory = std::function<TileSourcePtr(const nlohmann::json& json)>;

using ApiKeys = std::map<std::string, std::string>;
const std::string& getApiKey(const ApiKeys& keys, const std::string& name);

class JsonTileSourceFactoryRegistry
{
public:
	JsonTileSourceFactoryRegistry(const JsonTileSourceFactoryRegistryConfig& config);

	void addFactory(const std::string& name, JsonTileSourceFactory factory);
	const JsonTileSourceFactory& getFactory(const std::string& name) const;

	JsonTileSourceFactory wrapWithCacheSupport(JsonTileSourceFactory factory) const;
	JsonTileSourceFactory wrapWithProjectionSupport(JsonTileSourceFactory factory) const;

	const std::string& getCacheDirectory() const { return mCacheDirectory; }
	ApiKeys getApiKeys() const { return mApiKeys; }

private:
	const std::string mCacheDirectory;
	ApiKeys mApiKeys;
	std::map<std::string, JsonTileSourceFactory> mFactories;
};

void addDefaultFactories(JsonTileSourceFactoryRegistry& registry);

} // namespace vis
} // namespace skybolt
