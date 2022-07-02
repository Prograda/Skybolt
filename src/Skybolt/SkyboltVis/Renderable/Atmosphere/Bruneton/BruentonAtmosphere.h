/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/VisObject.h"
#include <osg/Group>
#include <osg/Texture2D>
#include <functional>

namespace skybolt {
namespace vis {

class BruentonAtmosphereGenerator;

struct ReyleighScatteringCoefficientRepresentation
{
	virtual ~ReyleighScatteringCoefficientRepresentation() {}

};

using ScatteringCoefficientCalculator = std::function<double(double wavelengthNanometers)>;

// @param coefficientPreLambdaPowFourDivide is divided by wavelength^4 to calculate the scattering coefficient.
ScatteringCoefficientCalculator createEarthReyleighScatteringCoefficientCalculator(double coefficientPreLambdaPowFourDivide);

// @param coefficientPreLambdaPowFourDivide is divided by wavelength^4 to calculate the scattering coefficient.
ScatteringCoefficientCalculator createTableReyleighScatteringCoefficientCalculator(const std::vector<double>& coefficient, const std::vector<double>& wavelengths);

struct BruentonAtmosphereConfig
{
	double bottomRadius = 6360000.0;
	double topRadius = 6420000.0;

	ScatteringCoefficientCalculator reyleighScatteringCoefficientCalculator; //!< Returns the reyleigh scattering coefficient for a given wavelength
	double rayleighScaleHeight = 8000.0;

	double mieScaleHeight = 1200.0;
	double mieAngstromAlpha = 0.0;
	double mieAngstromBeta = 5.328e-3;
	double mieSingleScatteringAlbedo = 0.9;
	double miePhaseFunctionG = 0.8;

	bool useEarthOzone = true;
};

class BruentonAtmosphere : public osg::Group
{
	friend class BruentonAtmosphereDrawCallback;
public:
	BruentonAtmosphere(const BruentonAtmosphereConfig& config);
	~BruentonAtmosphere();

	const osg::ref_ptr<osg::Texture>& getTransmittanceTexture() const;
	const osg::ref_ptr<osg::Texture>& getScatteringTexture() const;
	const osg::ref_ptr<osg::Texture>& getIrradianceTexture() const;

	osg::ref_ptr<osg::StateSet> getStateSet() const { return mStateSet; }

	static osg::Vec3f getSolarIrradiance();

private:
	osg::ref_ptr<osg::StateSet> mStateSet;
	osg::ref_ptr<BruentonAtmosphereGenerator> mGenerator;
	std::vector<osg::ref_ptr<osg::Uniform>> mUniforms;

	osg::ref_ptr<osg::Texture> mTransmittanceTexture;
	osg::ref_ptr<osg::Texture> mScatteringTexture;
	osg::ref_ptr<osg::Texture> mIrradianceTexture;
};

} // namespace vis
} // namespace skybolt