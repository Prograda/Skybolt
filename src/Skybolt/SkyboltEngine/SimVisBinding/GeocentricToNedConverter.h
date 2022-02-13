/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Component.h>
#include <SkyboltVis/RootNode.h>

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

	osg::Vec3d convertPosition(const sim::Vector3 &position) const;
	osg::Vec3d convertLocalPosition(const sim::Vector3 &position) const;
	sim::Vector3 convertLocalPosition(const osg::Vec3d &position) const;
	
	osg::Quat convert(const sim::Quaternion &ori) const;
	sim::Quaternion convert(const osg::Quat &ori) const;

private:
	sim::Matrix4 mNedBasis;
	sim::Matrix4 mNedBasisInverse;
	std::optional<PlanetPose> mPlanetPose;
};

} // namespace skybolt