/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/LatLonAlt.h>
#include <glm/glm.hpp>
#include <optional>

namespace skybolt {
namespace vis {

class LlaToNedConverter
{
public:
	LlaToNedConverter(const sim::LatLon& origin, double planetRadius);

	//! +x is north, +y is east, +z is down
	skybolt::sim::Vector3 latLonAltToCartesianNed(const sim::LatLonAlt& position) const;

	void setOrigin(const sim::LatLon& origin);

private:
	skybolt::sim::Vector3 mOrigin;
	skybolt::sim::Matrix3 mGeocentricToLtpOrientation;
	std::optional<double> mPlanetRadius;
};

} // namespace vis
} // namespace skybolt
