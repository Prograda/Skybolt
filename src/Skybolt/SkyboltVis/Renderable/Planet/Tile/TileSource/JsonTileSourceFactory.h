/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltCommon/Registry.h>
#include <SkyboltCommon/File/FileLocator.h>

#include <nlohmann/json.hpp>
#include <string>

namespace skybolt {
namespace vis {

struct JsonTileSourceFactoryRegistryConfig
{
	ImageFactoryPtr imageFactory;
	file::FileLocator fileLocator;
	std::string cacheDirectory;
	std::map<std::string, std::string> apiKeys;
};

using JsonTileSourceFactory = std::function<TileSourcePtr(const nlohmann::json& json)>;

using ApiKeys = std::map<std::string, std::string>;
const std::string& getApiKey(const ApiKeys& keys, const std::string& name);

class JsonTileSourceFactoryRegistry : public Registry
{
public:
	JsonTileSourceFactoryRegistry(const JsonTileSourceFactoryRegistryConfig& config);
	~JsonTileSourceFactoryRegistry() = default;

	void addDefaultFactories();

	void addFactory(const std::string& name, JsonTileSourceFactory factory);
	const JsonTileSourceFactory& getFactory(const std::string& name) const;

	JsonTileSourceFactory wrapWithCacheSupport(JsonTileSourceFactory factory) const;
	JsonTileSourceFactory wrapWithProjectionSupport(JsonTileSourceFactory factory) const;

	const std::string& getCacheDirectory() const { return mCacheDirectory; }
	ApiKeys getApiKeys() const { return mApiKeys; }

private:
	ImageFactoryPtr mImageFactory;
	file::FileLocator mFileLocator;
	const std::string mCacheDirectory;
	ApiKeys mApiKeys;
	std::map<std::string, JsonTileSourceFactory> mFactories;
};


} // namespace vis
} // namespace skybolt
