/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"

#include <optional>

namespace skybolt {

class GeocentricToNedConverter
{
public:
	GeocentricToNedConverter() : mNedBasis(sim::Matrix4()), mNedBasisInverse(sim::Matrix4()) {}

	struct PlanetPose
	{
		sim::Vector3 position;
		sim::Quaternion orientation;
	};

	void setOrigin(const sim::Vector3& origin, const std::optional<PlanetPose>& planetPose);

	std::optional<PlanetPose> getPlanetPose() const { return mPlanetPose; }

	glm::dvec3 convertPosition(const sim::Vector3 &position) const;
	glm::dvec3 convertLocalPosition(const sim::Vector3 &position) const;
	
	glm::dquat convert(const sim::Quaternion &ori) const;

private:
	sim::Matrix4 mNedBasis;
	sim::Matrix4 mNedBasisInverse;
	std::optional<PlanetPose> mPlanetPose;
};

} // namespace skybolt