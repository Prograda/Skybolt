/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltSim/Physics/Astronomy.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("calcJulianDate")
{
	double date = calcJulianDate(2018, 3, 2, 23.0372222222);
	CHECK(almostEqual(2458180.459884, date, 1e-6));
}

TEST_CASE("julianDateToYmd")
{
	int year, month, day;
	julianDateToYmd(2458511.75234, year, month, day);
	CHECK(year == 2019);
	CHECK(month == 1);
	CHECK(day == 28);
}

TEST_CASE("calcEarthAxialTilt")
{
	// Tested against http://radixpro.com/a4a-start/obliquity/

	double tilt = calcEarthAxialTilt(calcJulianDate(2016, 11, 2, 21.0 + 17.5 / 60.0));
	CHECK(almostEqual(0.409054590532109, tilt, 1e-8));
}


TEST_CASE("convertEclipticToEquatorial")
{
	// Tested against https://lambda.gsfc.nasa.gov/toolbox/tb_coordconv.cfm

	LatLon pos;
	pos.lon = 0.593412;
	pos.lat = 0.977384;
	pos = convertEclipticToEquatorial(calcJulianDate(1950, 1, 1, 0), pos);

	CHECK(almostEqual(-0.09244450543, pos.lon, 1e-6));
	CHECK(almostEqual(1.0865065547, pos.lat, 1e-6));
}

TEST_CASE("convertEquatorialToHorizontal")
{
	// Tested against https://www.satellite-calculations.com/Satellite/suncalc.htm

	LatLon equatorial;
	equatorial.lon = 5.999149429857;
	equatorial.lat = -0.120878223;

	LatLon observer;
	observer.lon = 0.785398;
	observer.lat = 0.20944;

	AzEl result = convertEquatorialToHorizontal(calcJulianDate(2018, 3, 3, 0), equatorial, observer);

	CHECK(almostEqual(1.51748109091, result.azimuth, 1e-4));
	CHECK(almostEqual(-0.843151108, result.elevation, 1e-4));
}

TEST_CASE("calcSunEclipticPosition")
{
	// Tested against https://midcdmz.nrel.gov/solpos/spa.html

	LatLon pos = calcSunEclipticPosition(2453371.5);
	pos = convertEclipticToEquatorial(2453371.5, pos);
	CHECK(almostEqual(-1.36832572428, pos.lon, 1e-3));
	CHECK(almostEqual(-0.40163693, pos.lat, 1e-3));
}

TEST_CASE("calcMoonEclipticPosition")
{
	// Tested against https://midcdmz.nrel.gov/solpos/spa.html

	double julianDate = calcJulianDate(2018, 3, 3, 0);

	LatLon pos = calcMoonEclipticPosition(julianDate);
	pos = convertEclipticToEquatorial(julianDate, pos);

	CHECK(almostEqual(3.08745254, pos.lon, 1e-3));
	CHECK(almostEqual(0.088121674, pos.lat, 1e-3));
}
