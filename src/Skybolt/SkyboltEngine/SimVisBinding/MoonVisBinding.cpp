/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MoonVisBinding.h"
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/Geocentric.h>

namespace skybolt {

MoonVisBinding::MoonVisBinding(JulianDateProvider dateProvider, osg::Uniform* moonPhaseUniform, const vis::RootNodePtr& visObject) :
	CelestialObjectVisBinding(dateProvider, sim::calcMoonEclipticPosition, visObject),
	mMoonPhaseUniform(moonPhaseUniform)
{}

void MoonVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	CelestialObjectVisBinding::syncVis(converter);

	double julianDate = mDateProvider();
	mMoonPhaseUniform->set((float)sim::calcMoonPhase(julianDate));
}

} // namespace skybolt