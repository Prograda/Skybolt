/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimVisBinding.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GeocentricToNedConverter.h>
#include <SkyboltVisOsg/OsgMathHelpers.h>
#include <SkyboltVisOsg/DefaultRootNode.h>

namespace skybolt {

using namespace sim;
using namespace vis;

SimpleSimVisBinding::SimpleSimVisBinding(const sim::Entity* entity) :
	mEntity(entity)
{
	assert(mEntity);
}

SimpleSimVisBinding::SimpleSimVisBinding(const sim::Entity* entity, const RootNodePtr& visObject,
					const glm::dvec3& visPositionOffset, const glm::dquat& visOrientationOffset) :
	mEntity(entity)
{
	assert(mEntity);
	addVisObject(visObject, visPositionOffset, visOrientationOffset);
}

void SimpleSimVisBinding::addVisObject(const vis::RootNodePtr& visObject, const glm::dvec3& visPositionOffset, const glm::dquat& visOrientationOffset)
{
	mVisObjects.push_back({	visObject, visPositionOffset, visOrientationOffset });
}

void SimpleSimVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	glm::dquat q = converter.convert(*sim::getOrientation(*mEntity));
	glm::dvec3 p = converter.convertPosition(*sim::getPosition(*mEntity));

	for (const auto& item : mVisObjects)
	{
		item.object->setOrientation(math::toOsgQuat(item.orientationOffset * q));
		item.object->setPosition(math::toOsgVec3d(p + glm::dvec3(q * item.positionOffset)));
	}
}

void syncVis(const sim::World& world, const GeocentricToNedConverter& converter)
{
	for (const sim::EntityPtr& entity : world.getEntities())
	{
		std::vector<SimVisBindingsComponentPtr> components = entity->getComponentsOfType<SimVisBindingsComponent>();
		for (const SimVisBindingsComponentPtr& component : components)
		{
			for (const SimVisBindingPtr& bindings : component->bindings)
			{
				bindings->syncVis(converter);
			}
		}
	}
}

} // namespace skybolt