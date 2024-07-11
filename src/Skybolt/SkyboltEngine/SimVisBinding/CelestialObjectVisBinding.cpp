/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CelestialObjectVisBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {

CelestialObjectVisBinding::CelestialObjectVisBinding(JulianDateProvider dateProvider, LatLonProvider eclipticDirectionProvider, const vis::RootNodePtr& visObject) :
	mDateProvider(dateProvider), mEclipticDirectionProvider(eclipticDirectionProvider), mVisObject(visObject) {}

void CelestialObjectVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	double julianDate = mDateProvider();
	sim::LatLon equatorial = sim::convertEclipticToEquatorial(julianDate, mEclipticDirectionProvider(julianDate));

	sim::AzEl azEl = convertEquatorialToHorizontal(julianDate, equatorial, sim::LatLon(skybolt::math::halfPiF(), 0));

	sim::Quaternion orientation = latLonToGeocentricLtpOrientation(sim::LatLon(-azEl.elevation + skybolt::math::halfPiF(), -azEl.azimuth));
	mVisObject->setOrientation(converter.convert(orientation));
}

} // namespace skybolt