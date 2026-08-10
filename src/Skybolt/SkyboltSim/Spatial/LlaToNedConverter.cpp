/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LlaToNedConverter.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include "SkyboltSim/Spatial/Geocentric.h"

namespace skybolt {
namespace vis {

LlaToNedConverter::LlaToNedConverter(const sim::LatLon& origin, double planetRadius) :
	mPlanetRadius(planetRadius)
{
	setOrigin(origin);
}

void LlaToNedConverter::setOrigin(const sim::LatLon& origin)
{
	mOrigin = sim::llaToGeocentric(sim::toLatLonAlt(origin, 0), sim::earthRadius());
	sim::Matrix3 mat = sim::geocentricToLtpOrientation(mOrigin);
	mGeocentricToLtpOrientation = glm::inverse(mat);
}

sim::Vector3 LlaToNedConverter::latLonAltToCartesianNed(const sim::LatLonAlt& position) const
{
	sim::Vector3 positionGeocentric = sim::llaToGeocentric(position, sim::earthRadius());
	return mGeocentricToLtpOrientation * (positionGeocentric - mOrigin);
}

} // namespace vis
} // namespace skybolt
