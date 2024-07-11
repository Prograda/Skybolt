/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BruentonAtmosphereGenerator.h"
#include <SkyboltVis/OsgStateSetHelpers.h>
#include <SkyboltVis/TextureGenerator/CompositingPipelineFactory.h>

using namespace atmosphere;

namespace skybolt {
namespace vis {

constexpr double kPi = 3.1415926;

bool use_combined_textures = true;
bool use_precomputed_luminance = false;

BruentonAtmosphereGenerator::BruentonAtmosphereGenerator(const BruentonAtmosphereGeneratorConfig& config) :
	mCompositingPipelineFactory(std::make_unique<CompositingPipelineFactory>())
{
	constexpr double kGroundAlbedo = 0.1;

	std::vector<double> ground_albedo;
	for (const double& wavelength : config.wavelengths)
	{
		ground_albedo.push_back(kGroundAlbedo);
	}

	atmosphere::Model model(config.wavelengths, config.solarIrradiance, config.sunAngularRadius,
		config.bottomRadius, config.topRadius, { config.rayleighLayer }, config.rayleighScattering,
		{ config.mieLayer }, config.mieScattering, config.mieExtinction, config.miePhaseFunctionG,
		config.ozoneDensity, config.absorptionExtinction, ground_albedo, config.maxSunZenithAngle,
		config.lengthUnitInMeters, use_precomputed_luminance ? 15 : 3,
		use_combined_textures, config.useHalfPrecision);

	atmosphere::Model::Pipeline pipeline = model.CreatePipeline();
	mTransmittanceTexture = pipeline.transmittanceTexture;
	mScatteringTexture = pipeline.scatteringTexture;
	mIrradianceTexture = pipeline.irradianceTexture;
	mOptionalSingleMieScatteringTexture = pipeline.optionalSingleMieScatteringTexture;

	addChild(mCompositingPipelineFactory->createCompositingPipeline(pipeline.precomputation));
}

BruentonAtmosphereGenerator::~BruentonAtmosphereGenerator()
{
}

} // namespace vis
} // namespace skybolt