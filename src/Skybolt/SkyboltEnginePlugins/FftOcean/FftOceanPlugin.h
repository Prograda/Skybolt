/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/Plugin/Plugin.h>
#include <SkyboltVis/VisFactory.h>
#ifdef BUILD_WITH_OSG
#include <SkyboltVisOsg/Renderable/Water/WaveHeightTextureGenerator.h>
#endif

namespace skybolt {

class FftOceanPlugin : public Plugin
{
public:
	FftOceanPlugin(const PluginConfig& config);

	~FftOceanPlugin() override;

private:
	vis::VisFactoryRegistryPtr mVisFactoryRegistry;
#ifdef BUILD_WITH_OSG
	std::shared_ptr<vis::WaveHeightTextureGeneratorFactory> mFactory;
#endif
};

namespace plugins {

	std::shared_ptr<Plugin> createFftOceanPlugin(const PluginConfig& config);
}

} // namespace skybolt