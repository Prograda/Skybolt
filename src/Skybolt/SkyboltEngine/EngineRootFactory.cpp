/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineRootFactory.h"
#include "EngineSettings.h"
#include "GetExecutablePath.h"
#include "Plugin/PluginHelpers.h"
#include <SkyboltCommon/File/OsDirectories.h>
#include <SkyboltCommon/Json/JsonHelpers.h>

#include <boost/log/trivial.hpp>

using namespace nlohmann;

namespace skybolt {

template <typename T>
T getOptionalNodeOrDefaultWithWarning(const json& j, const std::string& name, const T& defaultValue)
{
	auto i = j.find(name);
	if (i != j.end())
	{
		return i.value().get<T>();
	}
	BOOST_LOG_TRIVIAL(warning) << "Missing parameter '" << name << "'. Default value of '" << defaultValue << "' will be used.";
	return defaultValue;
}

std::unique_ptr<EngineRoot> EngineRootFactory::create(const boost::program_options::variables_map& params)
{
	std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(getDefaultPluginDirs());

	nlohmann::json settings = readEngineSettings(params);
	return create(enginePluginFactories, settings);
}

const std::string skyboltCacheDirEnvironmentVariable = "SKYBOLT_CACHE_DIR";

static file::Path getCacheDir()
{
	if (const char* dir = std::getenv(skyboltCacheDirEnvironmentVariable.c_str()); dir)
	{
		file::Path path(dir);
		if (!std::filesystem::exists(path))
		{
			BOOST_LOG_TRIVIAL(error) << "Environment variable '" << skyboltCacheDirEnvironmentVariable << "' not set to a valid path. Falling back to default.";
		}
		return dir;
	}
	else
	{
		return (file::getAppUserDataDirectory("Skybolt") / "Cache");
	}
}

std::unique_ptr<EngineRoot> EngineRootFactory::create(const std::vector<PluginFactory>& pluginFactories, const json& settings)
{
	file::Path cacheDir = getCacheDir();
	BOOST_LOG_TRIVIAL(info) << "Using cache directory '" << cacheDir.string() << "'.";

	EngineRootConfig config;
	config.pluginFactories = pluginFactories;
	config.tileSourceFactoryRegistryConfig.apiKeys = readNameMap<std::string>(settings, "tileApiKeys");
	config.tileSourceFactoryRegistryConfig.cacheDirectory = cacheDir.string();
	config.engineSettings = settings;
	return std::make_unique<EngineRoot>(config);
}

std::vector<std::string> EngineRootFactory::getDefaultPluginDirs()
{
	std::vector<std::string> pluginDirs = { getExecutablePath().append("plugins").string() };
	if (const char* path = std::getenv("SKYBOLT_PLUGINS_PATH"); path)
	{
		auto paths = file::splitByPathListSeparator(std::string(path));
		pluginDirs.insert(pluginDirs.begin(), paths.begin(), paths.end());
	}
	return pluginDirs;
}

} // namespace skybolt