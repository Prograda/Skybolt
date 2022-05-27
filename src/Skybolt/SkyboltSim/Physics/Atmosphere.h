/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/SimMath.h"

namespace skybolt {
namespace sim {

class Atmosphere
{
public:
    Atmosphere(double tempSeaLevel, double pressureSeaLevel, double tempLapsRate,
		double g, double molarMass);

	double getDensity(double altitude) const;

private:
	double m_lapseRateOnTemp; // L / T0
	double m_exponent;
	double m_tempSeaLevel;
	double m_pressureSeaLevel;
	double m_tempLapsRate;
	double m_molarMass;
    static const double m_universalGasConst;
};

Atmosphere createEarthAtmosphere();

} // namespace skybolt
} // namespace sim