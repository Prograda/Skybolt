/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonComponent.h"
#include <SkyboltEngine/Plugin/Plugin.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

namespace skybolt {

using namespace sim;

const std::string pythonComponentName = "python";

class PythonComponentPlugin : public Plugin
{
public:
	PythonComponentPlugin(const PluginConfig& config) :
		mComponentFactoryRegistry(config.simComponentFactoryRegistry)
	{
		auto factory = std::make_shared<ComponentFactoryFunctionAdapter>([](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			return std::make_shared<PythonComponent>(entity, context.julianDateProvider, json.at("module").get<std::string>(), json.at("class").get<std::string>());
		});

		mComponentFactoryRegistry->insert(std::make_pair(pythonComponentName, factory));
	}

	~PythonComponentPlugin()
	{
		mComponentFactoryRegistry->erase(pythonComponentName);
	}

private:
	ComponentFactoryRegistryPtr mComponentFactoryRegistry;
};

namespace plugins {

	std::shared_ptr<Plugin> createEnginePlugin(const PluginConfig& config)
	{
		return std::make_shared<PythonComponentPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEnginePlugin,
		createEnginePlugin
	)
}

} // namespace skybolt