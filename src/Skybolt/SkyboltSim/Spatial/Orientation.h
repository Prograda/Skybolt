/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "LatLonAlt.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"
#include <memory>

namespace skybolt {
namespace sim {

struct Orientation
{
	enum Type
	{
		TypeGeocentric,
		TypeLtpNed //! Local-tangent-plane North-east-down
	};

	const Type type;

	Orientation(Type type) : type(type) {}

	virtual ~Orientation() {}
};

struct GeocentricOrientation : Orientation //! Local-tangent-plane North-east-down
{
	GeocentricOrientation(const Quaternion& orientation) : Orientation(TypeGeocentric), orientation(orientation) {}

	Quaternion orientation;
};

struct LtpNedOrientation : Orientation //! Local-tangent-plane North-east-down
{
	LtpNedOrientation(const Quaternion& orientation) : Orientation(TypeLtpNed), orientation(orientation) {}

	Quaternion orientation;
};

GeocentricOrientation toGeocentric(const Orientation& orientation, const LatLon& latLon);
LtpNedOrientation toLtpNed(const Orientation& orientation, const LatLon& latLon);

} // namespace skybolt
} // namespace sim