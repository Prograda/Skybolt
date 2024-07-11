/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimVisBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltVis/RootNode.h>

namespace skybolt {

using namespace sim;
using namespace vis;

SimpleSimVisBinding::SimpleSimVisBinding(const sim::Entity* entity) :
	mEntity(entity)
{
	assert(mEntity);
}

SimpleSimVisBinding::SimpleSimVisBinding(const sim::Entity* entity, const RootNodePtr& visObject,
					const osg::Vec3d& visPositionOffset, const osg::Quat& visOrientationOffset) :
	mEntity(entity)
{
	assert(mEntity);
	addVisObject(visObject, visPositionOffset, visOrientationOffset);
}

void SimpleSimVisBinding::addVisObject(const vis::RootNodePtr& visObject, const osg::Vec3d& visPositionOffset, const osg::Quat& visOrientationOffset)
{
	mVisObjects.push_back({	visObject, visPositionOffset, visOrientationOffset });
}

void SimpleSimVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	osg::Quat q = converter.convert(*sim::getOrientation(*mEntity));
	osg::Vec3d p = converter.convertPosition(*sim::getPosition(*mEntity));

	for (const auto& item : mVisObjects)
	{
		item.object->setOrientation(item.orientationOffset * q);
		item.object->setPosition(p + osg::Vec3d(q * item.positionOffset));
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