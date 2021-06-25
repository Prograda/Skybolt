/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Physics/Orbit.h>
#include <SkyboltCommon/NumericComparison.h>
#include <catch2/catch.hpp>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("Convert ECEF to Orbital Elements, zero velocity")
{
	// Validated against http://orbitsimulator.com/formulas/OrbitalElements.html

	CreateOrbitFromEclipticCoordinatesArgs args;
	args.bodyMass = 1000;
	args.bodyPosition = Vector3(7e6, 0, 0);
	args.bodyVelocity = Vector3(0, 0, 0);
	args.planetMass = 5.972e24;

	Orbit orbit = createOrbitFromEclipticCoordinates(args);

	CHECK(almostEqual(3500000.0, orbit.semiMajorAxis, 0.1));
	CHECK(almostEqual(1.0, orbit.eccentricity, 1e-7));
	CHECK(almostEqual(0.0, orbit.inclination, 1e-7));
	CHECK(almostEqual(0.0, orbit.rightAscension, 1e-7));
	CHECK(almostEqual(skybolt::math::piD(), orbit.argumentOfPeriapsis, 1e-7));
	CHECK(almostEqual(skybolt::math::piD(), orbit.trueAnomaly, 1e-7));
}


TEST_CASE("Convert ECEF to Orbital Elements, degenerate node vector")
{
	// Validated against http://orbitsimulator.com/formulas/OrbitalElements.html

	CreateOrbitFromEclipticCoordinatesArgs args;
	args.bodyMass = 1000;
	args.bodyPosition = Vector3(7e6, 0, 0);
	args.bodyVelocity = Vector3(0, 1000, 0);
	args.planetMass = 5.972e24;

	Orbit orbit = createOrbitFromEclipticCoordinates(args);

	CHECK(almostEqual(3531013.6, orbit.semiMajorAxis, 0.1));
	CHECK(almostEqual(0.98243358059619, orbit.eccentricity, 1e-7));
	CHECK(almostEqual(0.0, orbit.inclination, 1e-7));
	CHECK(almostEqual(0.0, orbit.rightAscension, 1e-7));
	CHECK(almostEqual(skybolt::math::piD(), orbit.argumentOfPeriapsis, 1e-7));
	CHECK(almostEqual(skybolt::math::piD(), orbit.trueAnomaly, 1e-7));
}

TEST_CASE("Convert ECEF to Orbital Elements, non-trivial case")
{
	// Validated against http://orbitsimulator.com/formulas/OrbitalElements.html

	CreateOrbitFromEclipticCoordinatesArgs args;
	args.bodyMass = 1000;
	args.bodyPosition = Vector3(7e6, 1e6, 3e6);
	args.bodyVelocity = Vector3(7e3, 5e3, 1e3);
	args.planetMass = 5.972e24;

	Orbit orbit = createOrbitFromEclipticCoordinates(args);

	double epsilonFraction = 0.001;

	CHECK(almostEqualFracEpsilon(13856905.3, orbit.semiMajorAxis, epsilonFraction));
	CHECK(almostEqualFracEpsilon(0.887144943721, orbit.eccentricity, epsilonFraction));
	CHECK(almostEqualFracEpsilon(0.615479708668, orbit.inclination, epsilonFraction));
	CHECK(almostEqualFracEpsilon(3.92699, orbit.rightAscension, epsilonFraction));
	CHECK(almostEqualFracEpsilon(0.060614491053, orbit.argumentOfPeriapsis, epsilonFraction));
	CHECK(almostEqualFracEpsilon(2.338003764345002, orbit.trueAnomaly, epsilonFraction));
}
