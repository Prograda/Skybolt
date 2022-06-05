/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <boost/dll/import.hpp>
#include <boost/log/trivial.hpp>
#include <filesystem>
#include <functional>

namespace skybolt {

std::vector<std::filesystem::path> getAllPluginFilepathsInDirectory(const std::string& directory);

template <class PluginT, class PluginConfigT>
std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> loadPluginFactories(const std::string& directoryPath)
{
	namespace dll = boost::dll;

	std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> result;

	std::vector<std::filesystem::path> pluginFilepaths = getAllPluginFilepathsInDirectory(directoryPath);

	for (const std::filesystem::path& path : pluginFilepaths)
	{
		typedef std::shared_ptr<PluginT>(CreatePluginFunction)(const PluginConfigT&);

		boost::filesystem::path boostPath(path.string());

		try
		{
			dll::shared_library lib(boostPath, dll::load_mode::default_mode);
			if (lib.has(PluginT::factorySymbolName()))
			{
				std::function<CreatePluginFunction> creator = boost::dll::import_alias<CreatePluginFunction>(
					boostPath,
					PluginT::factorySymbolName(),
					dll::load_mode::default_mode
				);


				BOOST_LOG_TRIVIAL(info) << "Loaded plugin: " << boostPath.leaf().string();

				result.push_back([=](const PluginConfigT& config) { return creator(config); });
			}
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error("Error loading plugin '" + path.string() + "': " + e.what());
		}
	}

	return result;
}

} // namespace skybolt