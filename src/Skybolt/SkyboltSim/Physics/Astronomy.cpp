/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Astronomy.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <cmath>

namespace skybolt {
namespace sim {

// From A Physically-Based Night Sky Model by Henrik Wann Jensen et al
// http://graphics.stanford.edu/~henrik/papers/nightsky/nightsky.pdf

double calcJulianDate(int year, int month, int day, double hour)
{
	if (month <= 2)
	{
		month = month + 12;
		year = year - 1;
	}

	return 1720996.5 - std::floor(year / 100.0) + std::floor(year / 400.0) + std::floor(365.25 * year)
		+ std::floor(30.6001 * (month + 1) + day) + hour / 24.0;
}

void julianDateToYmd(double julianDate, int& year, int& month, int& day)
{
	// From http://mathforum.org/library/drmath/view/51907.html
	int p = int(julianDate + 0.5) + 68569;
	int q = 4 * p / 146097;
	int r = p - (146097 * q + 3) / 4;
	int s = 4000 * (r + 1) / 1461001;
	int t = r - 1461 * s / 4 + 31;
	int u = 80 * t / 2447;
	int v = u / 11;

	year = 100 * (q - 49) + s + v;
	month = u + 2 - 12 * v;
	day = t - 2447 * u / 80;
}

void julianDateToHms(double julianDate, int& hour, int& minute, double& second)
{
	double hourF = std::fmod(double(julianDate - (int)julianDate) + 0.5, 1.0) * 24.0;
	hour = int(hourF);
	double minuteF = (hourF - hour) * 60.0;
	minute = int(minuteF);
	second = (minuteF - minute) * 60.0;
}

double calcEarthAxialTilt(double julianDate)
{
	double T = (julianDate - 2451545.0) / 36525;
	double T2 = T * T;
	return 0.4090928042223289 - 0.0002269655248114293 * T - 0.00000000286040071855 * T2 + 0.00000000878967203852 * T2*T;
}

LatLon convertEclipticToEquatorial(double julianDate, const LatLon& ecliptic)
{
	double epsilon = calcEarthAxialTilt(julianDate);

	// calculate trigonometric combinations of coordinates
	double sd = sin(epsilon) * std::sin(ecliptic.lon) * std::cos(ecliptic.lat) + cos(epsilon) * std::sin(ecliptic.lat);
	double cacd = std::cos(ecliptic.lon) * std::cos(ecliptic.lat);
	double sacd = std::cos(epsilon) * std::sin(ecliptic.lon) * std::cos(ecliptic.lat) - std::sin(epsilon) * std::sin(ecliptic.lat);

	// calculate coordinates
	LatLon equatorial;
	equatorial.lon = atan2(sacd, cacd);
	double r = sqrt(cacd * cacd + sacd * sacd);
	equatorial.lat = atan2(sd, r);
	return equatorial;
}

double calcHourAngleOfVernalEquinox(double julianDate)
{
	// Based on https://github.com/pytroll/pyorbital/blob/main/pyorbital/astronomy.py
	double T = (julianDate - 2451545.0) / 36525;
	double theta = 67310.54841 + T * (876600 * 3600.0 + 8640184.812866 + T * (0.093104 - T * 6.2 * 10e-6));

	return math::normalizeAngleTwoPi(theta * math::degToRadD() / 240.0);
}

double calcHourAngle(double julianDate, const LatLon& equatorial, const LatLon& observer)
{
	return calcHourAngleOfVernalEquinox(julianDate) + observer.lon - equatorial.lon;
}

AzEl convertEquatorialToHorizontal(double julianDate, const LatLon& equatorial, const LatLon& observer)
{
	double hourAngle = calcHourAngle(julianDate, equatorial, observer);

	double x = std::cos(hourAngle) * std::cos(equatorial.lat);
	double y = std::sin(hourAngle) * std::cos(equatorial.lat);
	double z = std::sin(equatorial.lat);

	double xp = x * std::sin(observer.lat) - z * std::cos(observer.lat);
	double yp = y;
	double zp = x * std::cos(observer.lat) + z * std::sin(observer.lat);

	AzEl horizontal;
	horizontal.azimuth = std::atan2(yp, xp) + skybolt::math::piD();
	horizontal.elevation = std::atan2(zp, sqrt(xp * xp + yp * yp));
	return horizontal;
}

LatLon calcSunEclipticPosition(double julianDate)
{
	double T = (julianDate - 2451545.0) / 36525;
	double M = 6.24 + 628.302 * T;

	LatLon r;
	r.lon = fmod(4.895048 + 628.331951 * T + (0.033417 - 0.000084 * T) * std::sin(M), skybolt::math::twoPiD());
	r.lat = 0;
	return r;
}

LatLon calcMoonEclipticPosition(double julianDate)
{
	double T = (julianDate - 2451545.0) / 36525;

	double lp = 3.8104 + 8399.7091 * T;
	double m = 6.2300 + 628.3019 * T;
	double f = 1.6280 + 8433.4663 * T;
	double mp = 2.3554 + 8328.6911 * T;
	double d = 5.1985 + 7771.3772 * T;

	LatLon r;
	r.lon = lp
		+ 0.1098 * std::sin(mp)
		+ 0.0222 * std::sin(2 * d - mp)
		+ 0.0115 * std::sin(2 * d)
		+ 0.0037 * std::sin(2 * mp)
		- 0.0032 * std::sin(m)
		- 0.0020 * std::sin(2 * f)
		+ 0.0010 * std::sin(2 * d - 2 * mp)
		+ 0.0010 * std::sin(2 * d - m - mp)
		+ 0.0009 * std::sin(2 * d + mp)
		+ 0.0008 * std::sin(2 * d - m)
		+ 0.0007 * std::sin(mp - m)
		- 0.0006 * std::sin(d)
		- 0.0005 * std::sin(m + mp);

	r.lat = 0.0895 * std::sin(f)
		+ 0.0049 * std::sin(mp + f)
		+ 0.0048 * std::sin(mp - f)
		+ 0.0030 * std::sin(2 * d - f)
		+ 0.0010 * std::sin(2 * d + f - mp)
		+ 0.0008 * std::sin(2 * d - f - mp)
		+ 0.0006 * std::sin(2 * d + f);

	return r;
}

double calcMoonPhase(double julianDate)
{
	const double newMoonDate = 2451549.5;
	return fmod((julianDate - newMoonDate) / 29.53, 1.0);
}

} // namespace sim
} // namespace skybolt
