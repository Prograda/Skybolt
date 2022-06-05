/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/Plugin/Plugin.h>

namespace skybolt {

class FftOceanPlugin : public Plugin
{
public:
	FftOceanPlugin(const PluginConfig& config);

	~FftOceanPlugin() override;

private:
	vis::VisFactoryRegistryPtr mVisFactoryRegistry;
};

namespace plugins {

	std::shared_ptr<Plugin> createFftOceanPlugin(const PluginConfig& config);
}

} // namespace skybolt