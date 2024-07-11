/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include "Helpers/CaptureTexture.h"

#include <SkyboltVis/Renderable/Atmosphere/Bruneton/BruentonAtmosphereGenerator.h>
#include <SkyboltVis/Window/OffscreenViewer.h>
#include <SkyboltCommon/Stringify.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/State>
#include <osgDB/Registry>

using namespace skybolt;
using namespace vis;

static void registerResourceSearchPath(const std::string& path)
{
	auto& paths = osgDB::Registry::instance()->getDataFilePathList();
	if (std::find(paths.begin(), paths.end(), path) == paths.end())
	{
		paths.push_back(path);
	}
}

static void registerShaderSearchPath()
{
	registerResourceSearchPath("Assets/Core");
	registerResourceSearchPath(STRINGIFY(CMAKE_SOURCE_DIR) "/Assets/Core");
}

constexpr float epsilon = 0.00013f;

static BruentonAtmosphereGeneratorConfig createBruentonAtmosphereGeneratorConfig()
{
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
	// Wavelength independent solar irradiance "spectrum" (not physically
	// realistic, but was used in the original implementation).
	constexpr double kRayleigh = 1.24062e-6;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	constexpr double kMieAngstromBeta = 5.328e-3;
	constexpr double kMieSingleScatteringAlbedo = 0.9;

	BruentonAtmosphereGeneratorConfig c;
	c.sunAngularRadius = 0.00935 / 2.0;
	c.bottomRadius = 6360000.0;
	c.topRadius = 6420000.0;

	c.rayleighLayer = atmosphere::DensityProfileLayer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	c.mieLayer = atmosphere::DensityProfileLayer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	c.ozoneDensity.push_back(atmosphere::DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	c.ozoneDensity.push_back(atmosphere::DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	for (int l = kLambdaMin; l <= kLambdaMax; l += 10)
	{
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		c.wavelengths.push_back(l);
		c.solarIrradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		c.rayleighScattering.push_back(kRayleigh * pow(lambda, -4));
		c.mieScattering.push_back(mie * kMieSingleScatteringAlbedo);
		c.mieExtinction.push_back(mie);
		c.absorptionExtinction.push_back(kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10]);
	}

	c.miePhaseFunctionG = 0.8;
	c.useHalfPrecision = true;
	c.maxSunZenithAngle = (c.useHalfPrecision ? 102.0 : 120.0) * math::degToRadD();
	c.lengthUnitInMeters = 1000;

	return c;
}

TEST_CASE("Precomputed BruentonAtmosphere matches Bruenton's reference implementation")
{
	osg::setNotifyLevel(osg::WARN);
	registerShaderSearchPath();

	osg::ref_ptr<BruentonAtmosphereGenerator> atmosphere = new BruentonAtmosphereGenerator(createBruentonAtmosphereGeneratorConfig());

	int width = 64;
	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, width);
	viewer->getCamera()->addChild(atmosphere);
	osg::ref_ptr<ImageCaptureDrawCallback> callback = new ImageCaptureDrawCallback({
		ImageCaptureDrawCallback::CaptureItem(atmosphere->getTransmittanceTexture(), GL_FLOAT),
		ImageCaptureDrawCallback::CaptureItem(atmosphere->getScatteringTexture(), GL_FLOAT),
		ImageCaptureDrawCallback::CaptureItem(atmosphere->getIrradianceTexture(), GL_FLOAT)
	});
	viewer->getCamera()->setFinalDrawCallback(callback);

	// Generate
	viewer->frame();

	// Check transmittance texture
	{
		auto image = callback->capturedImages.at(0);
		const float* data = reinterpret_cast<const float*>(image->getDataPointer());
		CHECK(data[10000] == Approx(0.283131).margin(epsilon));
		CHECK(data[20000] == Approx(0.581288).margin(epsilon));
		CHECK(data[30000] == Approx(0.823351).margin(epsilon));
		CHECK(data[40000] == Approx(0.970999).margin(epsilon));
	}

	// Check scattering texture
	{
		auto image = callback->capturedImages.at(1);
		const float* data = reinterpret_cast<const float*>(image->getDataPointer());
		CHECK(data[100000] == Approx(0.093628).margin(epsilon));
		CHECK(data[200000] == Approx(0.085327).margin(epsilon));
		CHECK(data[300000] == Approx(0.190308).margin(epsilon));
		CHECK(data[400000] == Approx(0.0).margin(epsilon));
	}

	// Check irradiance texture
	{
		auto image = callback->capturedImages.at(2);
		const float* data = reinterpret_cast<const float*>(image->getDataPointer());
		CHECK(data[1000] == Approx(0.008781).margin(epsilon));
		CHECK(data[2000] == Approx(0.001163).margin(epsilon));
		CHECK(data[3000] == Approx(0.000130).margin(epsilon));
		CHECK(data[4000] == Approx(0.000000).margin(epsilon));
	}
}
