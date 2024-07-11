/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BruentonAtmosphere.h"
#include "BruentonAtmosphereGenerator.h"
#include "GlobalSamplerUnit.h"
#include "OsgStateSetHelpers.h"
#include "SkyboltVis/Renderable/Atmosphere/Bruneton/ThirdParty/model.h"
#include "SkyboltVis/TextureGenerator/TextureGeneratorCullCallback.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <osg/Camera>
#include <osg/StateSet>
#include <assert.h>

namespace skybolt {
namespace vis {

using atmosphere::DensityProfileLayer;

bool use_constant_solar_spectrum = false;

static osg::Vec3 Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, osg::Vec3 wavelength)
{
	return osg::Vec3(
		atmosphere::Interpolate(wavelengths, wavelength_function, wavelength.x()),
		atmosphere::Interpolate(wavelengths, wavelength_function, wavelength.y()),
		atmosphere::Interpolate(wavelengths, wavelength_function, wavelength.z()));
}

ScatteringCoefficientCalculator createEarthReyleighScatteringCoefficientCalculator(double coefficientPreLambdaPowFourDivide)
{
	return [=](double wavelengthNanometers) {
		return coefficientPreLambdaPowFourDivide * std::pow(wavelengthNanometers * 0.001, -4.0);
	};
}

// @param coefficientPreLambdaPowFourDivide is divided by wavelength^4 to calculate the scattering coefficient.
ScatteringCoefficientCalculator createTableReyleighScatteringCoefficientCalculator(const std::vector<double>& coefficient, const std::vector<double>& wavelengths)
{
	return [=](double wavelengthNanometers) {
		return atmosphere::Interpolate(wavelengths, coefficient, wavelengthNanometers);
	};
}

// Values from "Reference Solar Spectral Irradiance: ASTM G-173", ETR column
// (see http://rredc.nrel.gov/solar/spectra/am1.5/ASTMG173/ASTMG173.html),
// summed and averaged in each bin (e.g. the value for 360nm is the average
// of the ASTM G-173 values for all wavelengths between 360 and 370nm).
// Values in W.m^-2.
constexpr int kLambdaMin = 360;
constexpr int kLambdaMax = 830;
constexpr double kSolarIrradiance[48] = {
	1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
	1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
	1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
	1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
	1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
	1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
};

// Wavelength independent solar irradiance "spectrum" (not physically
// realistic, but was used in the original implementation).
constexpr double kConstantSolarIrradiance = 1.5;

BruentonAtmosphere::BruentonAtmosphere(const BruentonAtmosphereConfig& config) :
	mStateSet(new osg::StateSet)
{
	// Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
	// referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
	// each bin (e.g. the value for 360nm is the average of the original values
	// for all wavelengths between 360 and 370nm). Values in m^2.
	constexpr double kOzoneCrossSection[48] = {
	  1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
	  8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
	  1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
	  4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
	  2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
	  6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
	  2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};
	// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
	constexpr double kDobsonUnit = 2.687e20;
	// Maximum number density of ozone molecules, in m^-3 (computed so at to get
	// 300 Dobson units of ozone - for this we divide 300 DU by the integral of
	// the ozone density profile defined below, which is equal to 15km).
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;

	BruentonAtmosphereGeneratorConfig generatorConfig;

	generatorConfig.sunAngularRadius = 0.00935 / 2.0;

	generatorConfig.rayleighLayer = DensityProfileLayer(0.0, 1.0, -1.0 / config.rayleighScaleHeight, 0.0, 0.0);
	generatorConfig.mieLayer = DensityProfileLayer (0.0, 1.0, -1.0 / config.mieScaleHeight, 0.0, 0.0);
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	generatorConfig.ozoneDensity.push_back(
		DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	generatorConfig.ozoneDensity.push_back(
		DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	for (int lambda = kLambdaMin; lambda <= kLambdaMax; lambda += 10)
	{
		double lambdaMicroMeters = static_cast<double>(lambda) * 1e-3;  // micro-meters
		double mie = config.mieAngstromBeta / config.mieScaleHeight * pow(lambdaMicroMeters, -config.mieAngstromAlpha);
		generatorConfig.wavelengths.push_back(lambda);
		if (use_constant_solar_spectrum)
		{
			generatorConfig.solarIrradiance.push_back(kConstantSolarIrradiance);
		}
		else
		{
			generatorConfig.solarIrradiance.push_back(kSolarIrradiance[(lambda - kLambdaMin) / 10]);
		}

		generatorConfig.rayleighScattering.push_back(config.reyleighScatteringCoefficientCalculator(lambda));
		generatorConfig.mieScattering.push_back(mie * config.mieSingleScatteringAlbedo);
		generatorConfig.mieExtinction.push_back(mie);
		generatorConfig.absorptionExtinction.push_back(config.useEarthOzone ?
			kMaxOzoneNumberDensity * kOzoneCrossSection[(lambda - kLambdaMin) / 10] :
			0.0);
	}

	generatorConfig.bottomRadius = config.bottomRadius;
	generatorConfig.topRadius = config.topRadius;
	generatorConfig.miePhaseFunctionG = config.miePhaseFunctionG;

	generatorConfig.useHalfPrecision = true;
	generatorConfig.maxSunZenithAngle = (generatorConfig.useHalfPrecision ? 102.0 : 120.0) * math::degToRadD();

	mGenerator = osg::ref_ptr<BruentonAtmosphereGenerator>(new BruentonAtmosphereGenerator(generatorConfig));
	mGenerator->addCullCallback(new TextureGeneratorCullCallback());

	addChild(mGenerator);

	osg::Vec3 rgbWavelengths(atmosphere::Model::kLambdaR, atmosphere::Model::kLambdaG, atmosphere::Model::kLambdaB);

	mUniforms = {
		new osg::Uniform("solar_irradiance", Interpolate(generatorConfig.wavelengths, generatorConfig.solarIrradiance, rgbWavelengths)),
		new osg::Uniform("sun_angular_radius", float(generatorConfig.sunAngularRadius)),
		new osg::Uniform("bottom_radius", float(generatorConfig.bottomRadius / generatorConfig.lengthUnitInMeters)),
		new osg::Uniform("top_radius", float(generatorConfig.topRadius / generatorConfig.lengthUnitInMeters)),
		new osg::Uniform("rayleigh_scattering", Interpolate(generatorConfig.wavelengths, generatorConfig.rayleighScattering, rgbWavelengths) * generatorConfig.lengthUnitInMeters),
		new osg::Uniform("mie_scattering", Interpolate(generatorConfig.wavelengths, generatorConfig.mieScattering, rgbWavelengths) * generatorConfig.lengthUnitInMeters),
		new osg::Uniform("mie_phase_function_g", float(generatorConfig.miePhaseFunctionG)),
		new osg::Uniform("mu_s_min", float(std::cos(generatorConfig.maxSunZenithAngle)))
	};

	mStateSet->setTextureAttributeAndModes((int)GlobalSamplerUnit::Transmittance, mGenerator->getTransmittanceTexture());
	mStateSet->addUniform(createUniformSampler2d("transmittance_texture", (int)GlobalSamplerUnit::Transmittance));

	mStateSet->setTextureAttributeAndModes((int)GlobalSamplerUnit::Scattering, mGenerator->getScatteringTexture());
	mStateSet->addUniform(createUniformSampler3d("scattering_texture", (int)GlobalSamplerUnit::Scattering));

	mStateSet->setTextureAttributeAndModes((int)GlobalSamplerUnit::Irradiance, mGenerator->getIrradianceTexture());
	mStateSet->addUniform(createUniformSampler2d("irradiance_texture", (int)GlobalSamplerUnit::Irradiance));

	for (const auto& uniform : mUniforms)
	{
		mStateSet->addUniform(uniform);
	}

	mTransmittanceTexture = mGenerator->getTransmittanceTexture();
	mScatteringTexture = mGenerator->getScatteringTexture();
	mIrradianceTexture = mGenerator->getIrradianceTexture();
}

BruentonAtmosphere::~BruentonAtmosphere()
{
	if (mGenerator)
	{
		removeChild(mGenerator.get());
	}
}

static osg::Vec3f calcSolarIrradiance()
{
	std::vector<double> wavelengths;
	std::vector<double> solarIrradiance;

	for (int lambda = kLambdaMin; lambda <= kLambdaMax; lambda += 10)
	{
		double lambdaMicroMeters = static_cast<double>(lambda) * 1e-3;  // micro-meters
		wavelengths.push_back(lambda);
		if (use_constant_solar_spectrum)
		{
			solarIrradiance.push_back(kConstantSolarIrradiance);
		}
		else
		{
			solarIrradiance.push_back(kSolarIrradiance[(lambda - kLambdaMin) / 10]);
		}
	}
	osg::Vec3 rgbWavelengths(atmosphere::Model::kLambdaR, atmosphere::Model::kLambdaG, atmosphere::Model::kLambdaB);
	return Interpolate(wavelengths, solarIrradiance, rgbWavelengths);
}

osg::Vec3f BruentonAtmosphere::getSolarIrradiance()
{
	static osg::Vec3f solarIrradiance = calcSolarIrradiance();
	return solarIrradiance;
}

const osg::ref_ptr<osg::Texture>& BruentonAtmosphere::getTransmittanceTexture() const
{
	return mTransmittanceTexture;
}

const osg::ref_ptr<osg::Texture>& BruentonAtmosphere::getScatteringTexture() const
{
	return mScatteringTexture;
}

const osg::ref_ptr<osg::Texture>& BruentonAtmosphere::getIrradianceTexture() const
{
	return mIrradianceTexture;
}

} // namespace vis
} // namespace skybolt