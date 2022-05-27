/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Atmosphere.h"
#include <algorithm>
#include <cmath>

namespace skybolt {
namespace sim {

const double Atmosphere::m_universalGasConst = 8.31447;

Atmosphere::Atmosphere(double tempSeaLevel, double pressureSeaLevel,
	double tempLapsRate, double g, double molarMass) :
    m_tempSeaLevel(tempSeaLevel),
    m_pressureSeaLevel(pressureSeaLevel),
    m_tempLapsRate(tempLapsRate),
    m_lapseRateOnTemp(tempLapsRate / tempSeaLevel),
    m_molarMass(molarMass)
{
    m_exponent = g * molarMass / (m_universalGasConst * tempLapsRate);
}

double Atmosphere::getDensity(double altitude) const
{
	double p = m_pressureSeaLevel * (pow(std::max(double(0.0), double(1.0) - m_lapseRateOnTemp * altitude), m_exponent));
	double T = m_tempSeaLevel - m_tempLapsRate * altitude;
    return std::max(0.0, p * m_molarMass / (m_universalGasConst * T));
}

Atmosphere createEarthAtmosphere()
{
	return Atmosphere(288.15, 101300, 0.0065, 9.8, 0.0289644);
}

} // namespace sim
} // namespace skybolt
