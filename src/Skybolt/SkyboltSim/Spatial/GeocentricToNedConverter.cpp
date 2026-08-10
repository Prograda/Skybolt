/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "GeocentricToNedConverter.h"
#include "Geocentric.h"
#include <glm/gtc/matrix_transform.hpp>

namespace skybolt {

using namespace sim;

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

glm::dvec3 GeocentricToNedConverter::convertPosition(const sim::Vector3 &position) const
{
	glm::dvec4 p = mNedBasisInverse * glm::dvec4(position, 1.0);
	return glm::dvec3(p.x, p.y, p.z);
}

glm::dvec3 GeocentricToNedConverter::convertLocalPosition(const sim::Vector3 &position) const
{
	sim::Vector3 p = glm::dmat3(mNedBasisInverse) * position;
	return glm::dvec3(p.x, p.y, p.z);
}

glm::dquat GeocentricToNedConverter::convert(const sim::Quaternion &ori) const
{
	return sim::Quaternion(mNedBasisInverse) * ori;
}

} // namespace skybolt