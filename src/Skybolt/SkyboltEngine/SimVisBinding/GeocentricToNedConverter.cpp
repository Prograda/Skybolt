/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltVis/RootNode.h>
#include <glm/gtc/matrix_transform.hpp>

namespace skybolt {

using namespace sim;
using namespace vis;

void GeocentricToNedConverter::setOrigin(const sim::Vector3& origin, const std::optional<PlanetPose>& planetPose)
{
	sim::Vector3 posRelPlanet = origin;
	if (planetPose)
	{
		posRelPlanet -= planetPose->position;
	}

	double length = glm::length(posRelPlanet);
	Vector3 down = length > 0 ? -posRelPlanet / length : Vector3(-1,0,0);
	Vector3 north = Vector3(0, 0, 1);
	if (planetPose)
	{
		north = planetPose->orientation * north;
	}
	
	Vector3 eastUnnormalized = glm::cross(down, north);
	length = glm::length(eastUnnormalized);
	Vector3 east = length > 0 ? eastUnnormalized / length : Vector3(0,1,0);
	north = glm::cross(east, down);

	Matrix3 rotation(north, east, down);
	mNedBasis = Matrix4(rotation);
	mNedBasis[3] = glm::dvec4(origin, 1);

	mNedBasisInverse = glm::inverse(mNedBasis);

	mPlanetPose = planetPose;
}

osg::Vec3d GeocentricToNedConverter::convertPosition(const sim::Vector3 &position) const
{
	glm::dvec4 p = mNedBasisInverse * glm::dvec4(position, 1.0);
	return osg::Vec3d(p.x, p.y, p.z);
}

osg::Vec3d GeocentricToNedConverter::convertLocalPosition(const sim::Vector3 &position) const
{
	sim::Vector3 p = glm::dmat3(mNedBasisInverse) * position;
	return osg::Vec3d(p.x, p.y, p.z);
}

sim::Vector3 GeocentricToNedConverter::convertLocalPosition(const osg::Vec3d &position) const
{
	sim::Vector3 p = sim::Vector3(position.x(), position.y(), position.z());
	return glm::dmat3(mNedBasis) * p;
}

osg::Quat GeocentricToNedConverter::convert(const sim::Quaternion &ori) const
{
	sim::Quaternion q = sim::Quaternion(mNedBasisInverse) * ori;
	return osg::Quat(q.x, q.y, q.z, q.w);
}

sim::Quaternion GeocentricToNedConverter::convert(const osg::Quat &ori) const
{
	sim::Quaternion q (ori.x(), ori.y(), ori.z(), ori.w());
	return sim::Quaternion(mNedBasis) * q;
}

} // namespace skybolt