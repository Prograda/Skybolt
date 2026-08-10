/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FftOceanPlugin.h"

#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/EngineRoot.h>

#ifdef BUILD_WITH_OSG
#include "FftOceanWaveHeightTextureGenerator.h"
#include <SkyboltVisOsg/Renderable/Water/SimpleWaveHeightTextureGenerator.h>
#endif

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {

#ifdef BUILD_WITH_OSG
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
#endif

FftOceanPlugin::FftOceanPlugin(const PluginConfig& config)
{
	mVisFactoryRegistry = valueOrThrowException(getExpectedRegistry<vis::VisFactoryRegistry>(*config.engineRoot->factoryRegistries));
#ifdef BUILD_WITH_OSG
	// If using the default SimpleWaveHeightTextureGeneratorFactory, or no factory, then use this plugin instead.
	// Otherwise anothe plugin is already being used and it should take precedent, so do nothing.
	auto currentFactory = mVisFactoryRegistry->getFirstItemOfType<vis::WaveHeightTextureGeneratorFactory>();
	if (!currentFactory || dynamic_cast<vis::SimpleWaveHeightTextureGeneratorFactory*>(currentFactory.get()))
	{
		mVisFactoryRegistry->removeItem(currentFactory);
		mFactory = std::make_shared<FftOceanWaveHeightTextureGeneratorFactory>();
		mVisFactoryRegistry->addItem(mFactory);
	}
#endif
}

FftOceanPlugin::~FftOceanPlugin()
{
#ifdef BUILD_WITH_OSG
	mVisFactoryRegistry->removeItem(mFactory);
#endif
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