/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Orbit.h"

namespace skybolt {
namespace sim {

const double gravitationalConst = 6.6726e-11;

Orbit createOrbitFromEclipticCoordinates(const CreateOrbitFromEclipticCoordinatesArgs& args)
{
	Orbit orbit;

	// From https://space.stackexchange.com/questions/1904/how-to-programmatically-calculate-orbital-elements-using-position-velocity-vecto

	double mu = gravitationalConst * (args.planetMass + args.bodyMass);
	//double reducedMass = (state.bodyMass * state.planetMass) / (state.bodyMass + state.planetMass);

	Vector3 velocity = args.bodyVelocity;
	double speed = glm::length(velocity);
	Vector3 position = args.bodyPosition;
	Vector3 angularMom = glm::cross(position, velocity);

	double radius = glm::length(position);

	orbit.semiMajorAxis = 1.0 / (2.0 / radius - speed*speed / mu);

	Vector3 nodeVec = glm::cross(Vector3(0, 0, 1), angularMom);
	Vector3 eccentricityVec = ((speed*speed - (mu / radius))*position - glm::dot(position, velocity)*velocity) / mu;
	orbit.eccentricity = glm::length(eccentricityVec);

	double angularMomLength = glm::length(angularMom);
	if (angularMomLength > 0.0)
	{
		orbit.inclination = acos(angularMom.z / angularMomLength);

		double nodeVecLength = glm::length(nodeVec);

		if (nodeVecLength == 0.0)
		{
			orbit.rightAscension = 0;
			orbit.argumentOfPeriapsis = acos(eccentricityVec.x / orbit.eccentricity);
		}
		else
		{
			orbit.rightAscension = acos(nodeVec.x / nodeVecLength);

			if (nodeVec.y < 0)
			{
				orbit.rightAscension = skybolt::math::twoPiD() - orbit.rightAscension;
			}

			orbit.argumentOfPeriapsis = acos(glm::dot(nodeVec, eccentricityVec) / (nodeVecLength*orbit.eccentricity));

			if (eccentricityVec.z < 0)
			{
				orbit.argumentOfPeriapsis = skybolt::math::twoPiD() - orbit.argumentOfPeriapsis;
			}
		}

		orbit.trueAnomaly = acos(glm::dot(eccentricityVec, position) / (orbit.eccentricity*radius));

		if (glm::dot(position, velocity) < 0)
		{
			orbit.trueAnomaly = skybolt::math::twoPiD() - orbit.trueAnomaly;
		}
	}
	else // degnerate orbit
	{
		orbit.inclination = 0.0;
		orbit.rightAscension = 0;
		orbit.argumentOfPeriapsis = skybolt::math::piD();
		orbit.trueAnomaly = skybolt::math::piD();
	}

	return orbit;
}

std::string to_string(const Orbit& orbit)
{
	return
		"semiMajorAxis: " + std::to_string(orbit.semiMajorAxis) +
		" eccentricity: " + std::to_string(orbit.eccentricity) +
		" inclination: " + std::to_string(orbit.inclination) +
		" rightAscension: " + std::to_string(orbit.rightAscension) +
		" argumentOfPeriapsis: " + std::to_string(orbit.argumentOfPeriapsis) +
		" trueAnomaly: " + std::to_string(orbit.trueAnomaly);
}

OrbitTraverser::OrbitTraverser(Orbit* orbit) :
	mOrbit(orbit)
{
	assert(mOrbit);
	mSemiLatusRectum = orbit->semiMajorAxis * (1.0 - orbit->eccentricity * orbit->eccentricity);
}

DistReal OrbitTraverser::getRadius(DistReal theta) const
{
	double denominator = 1 + mOrbit->eccentricity * cos(theta);
	if (denominator <= 0.0)
		return -1.0;
	return mSemiLatusRectum / denominator;
}

} // namespace sim
} // namespace skybolt