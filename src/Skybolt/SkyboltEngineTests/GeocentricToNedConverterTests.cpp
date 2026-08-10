/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltSim/Spatial/GeocentricToNedConverter.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/NumericComparison.h>


using namespace skybolt;

constexpr float epsilon = 1e-8f;

const GeocentricToNedConverter::PlanetPose identityPlanetPose = [] {
	GeocentricToNedConverter::PlanetPose p;
	p.position = math::dvec3Zero();
	p.orientation = math::dquatIdentity();
	return p;
}();

TEST_CASE("NED position is zero at origin")
{
	sim::Vector3 origin(1, 2, 3);

	GeocentricToNedConverter::PlanetPose pose = identityPlanetPose;
	pose.position = sim::Vector3(10, 20, 30);
	pose.orientation = glm::angleAxis(math::halfPiD(), sim::Vector3(0, 1, 0));

	GeocentricToNedConverter converter;
	converter.setOrigin(origin, pose);

	glm::dvec3 ned = converter.convertPosition(origin);

	check(glm::dvec3(0, 0, 0), ned, epsilon);
}

TEST_CASE("NED position is +X when north of origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10,0,0), identityPlanetPose);

	glm::dvec3 ned = converter.convertPosition(sim::Vector3(10, 0, 2));

	check(glm::dvec3(2,0,0), ned, epsilon);
}

TEST_CASE("NED position is +Y when east of origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0), identityPlanetPose);

	glm::dvec3 ned = converter.convertPosition(sim::Vector3(10, 2, 0));

	check(glm::dvec3(0, 2, 0), ned, epsilon);
}

TEST_CASE("NED position is +Z when below origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0), identityPlanetPose);

	glm::dvec3 ned = converter.convertPosition(sim::Vector3(8, 0, 0));

	check(glm::dvec3(0, 0, 2), ned, epsilon);
}

TEST_CASE("Geocentric up vector converts to NED down vector")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 10, 10), identityPlanetPose);

	glm::dvec3 ned = converter.convertLocalPosition(sim::Vector3(1, 1, 1));

	check(glm::dvec3(0, 0, -std::sqrt(3)), ned, epsilon);
}

TEST_CASE("Conversion accounts for planet position")
{
	GeocentricToNedConverter::PlanetPose pose = identityPlanetPose;
	pose.position = sim::Vector3(20, 0, 0); // displace the planet to put the origin on the opposite side

	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0), pose);

	// Test that a point to the east of a planet at (0,0,0) is now to the west
	glm::dvec3 ned = converter.convertPosition(sim::Vector3(10, 2, 0));
	check(glm::dvec3(0,-2,0), ned, epsilon);
}

TEST_CASE("Conversion accounts for planet orientation")
{
	GeocentricToNedConverter::PlanetPose pose = identityPlanetPose;
	pose.orientation = glm::angleAxis(math::piD(), sim::Vector3(0, 1, 0)); // Rotate planet 180 deg to swap poles

	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0), pose);

	// Test that a point to the east of the unrotated planet is now to the west
	glm::dvec3 ned = converter.convertPosition(sim::Vector3(10, 2, 0));
	check(glm::dvec3(0,-2,0), ned, epsilon);
}