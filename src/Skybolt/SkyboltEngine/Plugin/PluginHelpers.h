/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <boost/dll/import.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <functional>

namespace skybolt {

std::vector<boost::filesystem::path> getAllPluginFilepathsInDirectory(const std::string& directory);

template <class PluginT, class PluginConfigT>
std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> loadPluginFactories(const std::string& directoryPath)
{
	namespace dll = boost::dll;

	std::vector<std::function<std::shared_ptr<PluginT>(const PluginConfigT&)>> result;

	std::vector<boost::filesystem::path> pluginFilepaths = getAllPluginFilepathsInDirectory(directoryPath);

	for (const boost::filesystem::path& path : pluginFilepaths)
	{
		typedef std::shared_ptr<PluginT>(CreatePluginFunction)(const PluginConfigT&);

		dll::shared_library lib(path, dll::load_mode::default_mode);
		if (lib.has(PluginT::factorySymbolName()))
		{
			std::function<CreatePluginFunction> creator = boost::dll::import_alias<CreatePluginFunction>(
				path,
				PluginT::factorySymbolName(),
				dll::load_mode::default_mode
				);

			BOOST_LOG_TRIVIAL(info) << "Loaded plugin: " << path.leaf().string();

			result.push_back([=](const PluginConfigT& config) { return creator(config); });
		}
	}

	return result;
}

} // namespace skybolt