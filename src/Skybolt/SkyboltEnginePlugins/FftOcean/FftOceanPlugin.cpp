/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FftOceanPlugin.h"
#include "FftOceanWaveHeightTextureGenerator.h"

#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltVis/Renderable/Water/SimpleWaveHeightTextureGenerator.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {

class FftOceanWaveHeightTextureGeneratorFactory : public vis::WaveHeightTextureGeneratorFactory
{
public:
	std::unique_ptr<vis::WaveHeightTextureGenerator> create() const override
	{
		return std::make_unique<vis::FftOceanWaveHeightTextureGenerator>([&] {
			vis::FftOceanWaveHeightTextureGeneratorConfig c;
			c.textureWorldSize = 500; // FIXME: To avoid texture wrapping issues, Scene::mWrappedNoisePeriod divided by this should have no remainder;
			c.textureSizePixels = 512;
			return c;
			}());
	}
};

FftOceanPlugin::FftOceanPlugin(const PluginConfig& config)
{
	mVisFactoryRegistry = valueOrThrowException(getExpectedRegistry<vis::VisFactoryRegistry>(*config.engineRoot->factoryRegistries));

	// If using the default SimpleWaveHeightTextureGeneratorFactory, or no factory, then use this plugin instead.
	// Otherwise anothe plugin is already being used and it should take precedent, so do nothing.
	auto currentFactory = (*mVisFactoryRegistry)[vis::VisFactoryType::WaveHeightTextureGenerator];
	if (!currentFactory || dynamic_cast<vis::SimpleWaveHeightTextureGeneratorFactory*>(currentFactory.get()))
	{
		(*mVisFactoryRegistry)[vis::VisFactoryType::WaveHeightTextureGenerator] = std::make_shared<FftOceanWaveHeightTextureGeneratorFactory>();
	}
}

FftOceanPlugin::~FftOceanPlugin()
{
	mVisFactoryRegistry->erase(vis::VisFactoryType::WaveHeightTextureGenerator);
}

namespace plugins {

	std::shared_ptr<Plugin> createFftOceanPlugin(const PluginConfig& config)
	{
		return std::make_shared<FftOceanPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createFftOceanPlugin,
		createEnginePlugin
	)
}

} // namespace skybolt