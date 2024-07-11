/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "LatLonAlt.h"
#include "SkyboltSim/SimMath.h"
#include <memory>

namespace skybolt {
namespace sim {

struct Position
{
	enum Type
	{
		TypeGeocentric,
		TypeLatLonAlt,
		TypeNed
	};

	const Type type;

	Position(Type type) : type(type) {}

	virtual ~Position() {}
};

struct PositionRelPlanet : Position
{
};

struct GeocentricPosition : Position
{
	GeocentricPosition(const Vector3& position) : Position(TypeGeocentric), position(position) {}

	Vector3 position;
};

struct LatLonAltPosition : Position
{
	LatLonAltPosition(const LatLonAlt& position) : Position(TypeLatLonAlt), position(position) {}

	LatLonAlt position;
};

//! North-East-Down
struct NedPosition : Position
{
	NedPosition(const Vector3& position, std::shared_ptr<PositionRelPlanet>& origin) :
		Position(TypeNed), position(position), origin(origin) {}

	Vector3 position;
	std::shared_ptr<PositionRelPlanet> origin; //!< never null
};

GeocentricPosition toGeocentric(const Position& position);
LatLonAltPosition toLatLonAlt(const Position& position);

} // namespace skybolt
} // namespace sim