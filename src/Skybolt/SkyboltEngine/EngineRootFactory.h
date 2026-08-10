/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EngineRoot.h"
#include <SkyboltVis/SkyboltVisFwd.h>
#include <boost/program_options/variables_map.hpp>

namespace skybolt {

class EngineRootFactory
{
public:
	static std::unique_ptr<EngineRoot> create(const boost::program_options::variables_map& params, const vis::ImageFactoryPtr& imageFactory = createDefaultImageFactory());
	static std::unique_ptr<EngineRoot> create(const std::vector<PluginFactory>& pluginFactories, const nlohmann::json& settings, const vis::ImageFactoryPtr& imageFactory = createDefaultImageFactory());

	static std::vector<std::string> getDefaultPluginDirs();

	static vis::ImageFactoryPtr createDefaultImageFactory();
};

} // namespace skybolt