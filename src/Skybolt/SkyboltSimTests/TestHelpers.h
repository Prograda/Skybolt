/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Spatial/LatLonAlt.h>

inline bool almostEqual(const skybolt::sim::Vector3& a, const skybolt::sim::Vector3& b, double epsilon)
{
	return (skybolt::almostEqual(a.x, b.x, epsilon)
		&& skybolt::almostEqual(a.y, b.y, epsilon)
		&& skybolt::almostEqual(a.z, b.z, epsilon));
}

inline bool almostEqual(const skybolt::sim::LatLonAlt& a, const skybolt::sim::LatLonAlt& b, double epsilon)
{
	return (skybolt::almostEqual(a.lat, b.lat, epsilon)
		&& skybolt::almostEqual(a.lon, b.lon, epsilon)
		&& skybolt::almostEqual(a.alt, b.alt, epsilon));
}
