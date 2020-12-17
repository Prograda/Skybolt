/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisOrbits.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/OrbitComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Physics/Orbit.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltVis/ShaderProgramRegistry.h>

#include <osg/Depth>
#include <osg/Geode>
#include <osgText/Text>

namespace skybolt {

using namespace sim;

VisOrbits::VisOrbits(World* world, osg::Group* parent, const vis::Polyline::Params& params, JulianDateProvider julianDateProvider) :
	SimVisObjectsReflector<vis::PolylinePtr>(world, parent),
	mParams(params),
	mJulianDateProvider(julianDateProvider)
{
}

VisOrbits::~VisOrbits()
{
}

static boost::optional<Orbit> getOrbit(const Entity& entity, double julianDate)
{
	if (const auto& controller = entity.getFirstComponent<OrbitComponent>())
	{
		return controller->orbit;
	}
	else if (const auto& body = entity.getFirstComponent<DynamicBodyComponent>())
	{
		if (const auto& node = entity.getFirstComponent<Node>())
		{
			Vector3 planetPosition = math::dvec3Zero();

			Quaternion orientation = getEquatorialToEcefRotation(julianDate);

			CreateOrbitFromEclipticCoordinatesArgs args;
			args.planetMass = 5.972e24; // earth
			args.bodyPosition = orientation * (node->getPosition() - planetPosition);
			args.bodyVelocity = orientation * body->getLinearVelocity();
			args.bodyMass = body->getMass();
			return createOrbitFromEclipticCoordinates(args);
		}
	}
	return boost::none;
}

static bool hasOrbit(const Entity& entity)
{
	return entity.getFirstComponent<OrbitComponent>() || entity.getFirstComponent<DynamicBodyComponent>();
}

void VisOrbits::syncVis(const GeocentricToNedConverter& converter)
{
	double julianDate = mJulianDateProvider();

	for (const auto& entry : getObjectsMap())
	{
		Entity* entity = entry.first;
		vis::PolylinePtr polyline = entry.second;

		if (applyVisibility(*entity, polyline))
		{
			Orbit orbit = *getOrbit(*entity, julianDate);
			OrbitTraverser traverser(&orbit);

			osg::ref_ptr<osg::Vec3Array> points(new osg::Vec3Array);
			static const int sampleCount = 128;
			for (int i = 0; i < sampleCount; ++i)
			{
				double theta = double(i) / (double(sampleCount) - 1) * skybolt::math::twoPiD();
				double radius = traverser.getRadius(theta);
				points->push_back(osg::Vec3(cos(theta) * radius, sin(theta) * radius, 0));
			}

			polyline->setPoints(points);

			polyline->setPosition(converter.convertPosition(math::dvec3Zero()));

			Quaternion orientation = glm::angleAxis(orbit.rightAscension, Vector3(0, 0, -1)) * glm::angleAxis(orbit.inclination, Vector3(1, 0, 0)) * glm::angleAxis(orbit.argumentOfPeriapsis, Vector3(0, 0, -1));

			orientation = getEquatorialToEcefRotation(julianDate) * orientation;
			polyline->setOrientation(converter.convert(orientation));
		}
	}
}

boost::optional< vis::PolylinePtr> VisOrbits::createObject(const sim::EntityPtr& entity)
{
	if (hasOrbit(*entity))
	{
		return std::make_shared<vis::Polyline>(mParams);
	}
	return boost::none;
}

} // namespace skybolt