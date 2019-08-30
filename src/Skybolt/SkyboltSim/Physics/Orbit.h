/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <string>

namespace skybolt {
namespace sim {

struct Orbit
{
	double semiMajorAxis;
	double eccentricity;
	double inclination;
	double rightAscension; //! a.k.a longitude of nodes
	double argumentOfPeriapsis;
	double trueAnomaly;
};

std::string to_string(const Orbit& orbit);

struct CreateOrbitFromEclipticCoordinatesArgs
{
	double planetMass;
	double bodyMass;
	Vector3 bodyPosition;
	Vector3 bodyVelocity;
};

Orbit createOrbitFromEclipticCoordinates(const CreateOrbitFromEclipticCoordinatesArgs& args);

class OrbitTraverser
{
public:
	OrbitTraverser(Orbit* orbit);

	// returns -1 if orbit is invalid at theta
	DistReal getRadius(DistReal theta) const;

private:
	Orbit* mOrbit;
	double mSemiLatusRectum;
};

} // namespace skybolt
} // namespace sim