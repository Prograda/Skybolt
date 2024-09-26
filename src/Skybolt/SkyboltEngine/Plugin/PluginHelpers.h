/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Plugin.h"
#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <boost/dll/import.hpp>
#include <boost/log/trivial.hpp>
#include <filesystem>
#include <functional>
#include <set>

namespace skybolt {

std::vector<std::filesystem::path> getAllPluginFilepathsInDirectory(const std::string& directory);
std::vector<std::filesystem::path> getAllPluginFilepathsInDirectories(const std::vector<std::string>& directories);

using PluginCategoriesSupported = std::function<bool(const Plugin::Categories&)>;

bool pluginSupported(const boost::dll::shared_library& lib, const PluginCategoriesSupported& pluginCategoriesSupported = {});

template <class PluginT, class PluginConfigT>
std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> loadPluginFactories(const std::vector<std::filesystem::path>& pluginFilepaths, const PluginCategoriesSupported& pluginCategoriesSupported = {})
{
	namespace dll = boost::dll;

	std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> result;
	std::set<std::string> loadedPluginNames;

	for (const std::filesystem::path& path : pluginFilepaths)
	{
		boost::filesystem::path boostPath(path.string());
		std::string pluginName = boostPath.leaf().string();
		if (loadedPluginNames.find(pluginName) != loadedPluginNames.end())
		{
			BOOST_LOG_TRIVIAL(warning) << "Found plugin with same name as already loaded plugin '" << pluginName << "' in directory '" << boostPath.string() << "'. Ignoring the duplicate plugin.";
			continue;
		}

		try
		{
			dll::shared_library lib(boostPath, dll::load_mode::default_mode);
			if (pluginSupported(lib, pluginCategoriesSupported))
			{
				using CreatePluginFunction = Plugin::CreatePluginFunctionT<PluginT, PluginConfigT>;
				auto creator = boost::dll::import_alias<CreatePluginFunction>(
					lib,
					PluginT::factorySymbolName()
				);

				loadedPluginNames.insert(pluginName);
				BOOST_LOG_TRIVIAL(info) << "Loaded plugin: " << pluginName;

				result.push_back([=](const PluginConfigT& config) { return creator(config); });
			}
		}
		catch (const std::exception& e)
		{
			BOOST_LOG_TRIVIAL(error) << "Error loading plugin '" << path.string() << "': " << e.what();
		}
	}

	return result;
}

} // namespace skybolt