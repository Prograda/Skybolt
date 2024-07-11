/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EngineRoot.h"
#include <boost/program_options/variables_map.hpp>

namespace skybolt {

class EngineRootFactory
{
public:
	static std::unique_ptr<EngineRoot> create(const boost::program_options::variables_map& params);
	static std::unique_ptr<EngineRoot> create(const std::vector<PluginFactory>& pluginFactories, const nlohmann::json& settings);

	static std::vector<std::string> getDefaultPluginDirs();
};

} // namespace skybolt