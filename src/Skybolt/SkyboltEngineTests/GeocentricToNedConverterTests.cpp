/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>
#include <SkyboltCommon/NumericComparison.h>
#include <osg/Vec3d>

using namespace skybolt;

constexpr float epsilon = 1e-8f;

TEST_CASE("NED position is zero at origin")
{
	sim::Vector3 origin(10, 0, 0);

	GeocentricToNedConverter converter;
	converter.setOrigin(origin);

	osg::Vec3d ned = converter.convertPosition(origin);

	check(osg::Vec3d(0, 0, 0), ned, epsilon);
}

TEST_CASE("NED position is +X when north of origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10,0,0));

	osg::Vec3d ned = converter.convertPosition(sim::Vector3(10, 0, 2));

	check(osg::Vec3d(2,0,0), ned, epsilon);
}

TEST_CASE("NED position is +Y when east of origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0));

	osg::Vec3d ned = converter.convertPosition(sim::Vector3(10, 2, 0));

	check(osg::Vec3d(0, 2, 0), ned, epsilon);
}

TEST_CASE("NED position is +Z when below origin")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 0, 0));

	osg::Vec3d ned = converter.convertPosition(sim::Vector3(8, 0, 0));

	check(osg::Vec3d(0, 0, 2), ned, epsilon);
}

TEST_CASE("Geocentric up vector converts to NED down vector")
{
	GeocentricToNedConverter converter;
	converter.setOrigin(sim::Vector3(10, 10, 10));

	osg::Vec3d ned = converter.convertLocalPosition(sim::Vector3(1, 1, 1));

	check(osg::Vec3d(0, 0, -std::sqrt(3)), ned, epsilon);
}