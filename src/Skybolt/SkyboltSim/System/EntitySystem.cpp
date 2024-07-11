/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntitySystem.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"

namespace skybolt {
namespace sim {

EntitySystem::EntitySystem(World* world) :
	mWorld(world)
{
	assert(mWorld);
}

void EntitySystem::setSimTime(SecondsD newTime)
{
	for (const EntityPtr& entity : mWorld->getEntities())
	{
		entity->setSimTime(newTime);
	}
}

void EntitySystem::advanceWallTime(SecondsD newTime, SecondsD dt)
{
	for (const EntityPtr& entity : mWorld->getEntities())
	{
		entity->advanceWallTime(newTime, dt);
	}
}

void EntitySystem::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	for (const EntityPtr& entity : mWorld->getEntities())
	{
		entity->advanceSimTime(newTime, dt);
	}
}

void EntitySystem::update(UpdateStage stage)
{
	// Take a copy of the entities container so that the list doesn't change during timestep
	// due to entities being added or removed from the world.
	std::vector<EntityPtr> entities = mWorld->getEntities();

	for (const EntityPtr& entity : entities)
	{
		if (!entity->isDynamicsEnabled() &&
			(stage == UpdateStage::PreDynamicsSubStep || stage == UpdateStage::DynamicsSubStep || stage == UpdateStage::PostDynamicsSubStep))
		{
			continue;
		}

		if (entity->isDynamicsEnabled() && stage == UpdateStage::PreDynamicsSubStep)
		{
			// Apply gravity
			auto position = getPosition(*entity);
			auto body = entity->getFirstComponent<DynamicBodyComponent>();
			if (body)
			{
				Vector3 force = mWorld->calcGravity(*position, body->getMass());
				body->applyCentralForce(force);
			}
		}

		entity->update(stage);
	}
}

} // namespace sim
} // namespace skybolt