/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineRootFactory.h"
#include "EngineSettings.h"
#include "GetExecutablePath.h"
#include "Plugin/PluginHelpers.h"

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
	std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(getAllPluginFilepathsInDirectories(getDefaultPluginDirs()));

	nlohmann::json settings = readEngineSettings(params);
	return create(enginePluginFactories, settings);
}

std::unique_ptr<EngineRoot> EngineRootFactory::create(const std::vector<PluginFactory>& pluginFactories, const json& settings)
{
	EngineRootConfig config;
	config.pluginFactories = pluginFactories;
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