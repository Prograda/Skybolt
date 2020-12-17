/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ThirdParty/model.h"

#include <osg/Group>
#include <osg/Texture>
#include <vector>

namespace skybolt {
namespace vis {

struct BruentonAtmosphereGeneratorConfig
{
	double sunAngularRadius;

	//! The distance between the planet center and the bottom of the atmosphere.
	double bottomRadius;

	//! The distance between the planet center and the top of the atmosphere.
	double topRadius;

	std::vector<double> wavelengths;
	std::vector<double> solarIrradiance;
	std::vector<double> rayleighScattering;
	std::vector<double> mieScattering;
	std::vector<double> mieExtinction;
	std::vector<double> absorptionExtinction;

	atmosphere::DensityProfileLayer rayleighLayer;
	atmosphere::DensityProfileLayer mieLayer;
	std::vector<atmosphere::DensityProfileLayer> ozoneDensity;

	//! The asymetry parameter for the Cornette-Shanks phase function for the
	//! aerosols.
	double miePhaseFunctionG;

	bool useHalfPrecision;
	double maxSunZenithAngle;
	double lengthUnitInMeters = 1.0;
};

class BruentonAtmosphereGenerator : public osg::Group
{
public:
	BruentonAtmosphereGenerator(const BruentonAtmosphereGeneratorConfig& config);
	~BruentonAtmosphereGenerator();

	const osg::ref_ptr<osg::Texture>& getTransmittanceTexture() const { return mTransmittanceTexture; }
	const osg::ref_ptr<osg::Texture>& getScatteringTexture() const { return mScatteringTexture; }
	const osg::ref_ptr<osg::Texture>& getIrradianceTexture() const { return mIrradianceTexture; }
	const osg::ref_ptr<osg::Texture>& getOptionalSingleMieScatteringTexture() const { return mOptionalSingleMieScatteringTexture; }

private:
	std::unique_ptr<class CompositingPipelineFactory> mCompositingPipelineFactory;
	osg::ref_ptr<osg::Texture> mTransmittanceTexture;
	osg::ref_ptr<osg::Texture> mScatteringTexture;
	osg::ref_ptr<osg::Texture> mIrradianceTexture;
	osg::ref_ptr<osg::Texture> mOptionalSingleMieScatteringTexture;
};

} // namespace vis
} // namespace skybolt