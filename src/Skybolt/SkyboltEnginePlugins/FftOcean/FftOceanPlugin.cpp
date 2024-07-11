/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FftOceanPlugin.h"
#include "FftOceanWaveHeightTextureGenerator.h"

#include <SkyboltCommon/Math/MathUtility.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {

class FftOceanWaveHeightTextureGeneratorFactory : public vis::WaveHeightTextureGeneratorFactory
{
public:
	std::unique_ptr<vis::WaveHeightTextureGenerator> create(float textureWorldSize, const glm::vec2& normalizedFrequencyRange) const override
	{
		return std::make_unique<vis::FftOceanWaveHeightTextureGenerator>(textureWorldSize, normalizedFrequencyRange);
	}
};

FftOceanPlugin::FftOceanPlugin(const PluginConfig& config) :
	mVisFactoryRegistry(config.visFactoryRegistry)
{
	(*mVisFactoryRegistry)[vis::VisFactoryType::WaveHeightTextureGenerator] = std::make_shared<FftOceanWaveHeightTextureGeneratorFactory>();
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