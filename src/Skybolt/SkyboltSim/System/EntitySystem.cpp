/* Copyright 2012-2020 Matthew Reid
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

void EntitySystem::updatePreDynamics(const System::StepArgs& args)
{
	// Take a copy of the entities container so that the list doesn't change during timestep,
	// even if entities are added to or removed from the world.
	mEntities = mWorld->getEntities();

	for (const EntityPtr& entity : mEntities)
	{
		if (entity->isDynamicsEnabled())
		{
			entity->updatePreDynamics(args.dtSim, args.dtWallClock);
		}
	}
}

void EntitySystem::updatePreDynamicsSubstep(double dtSubstep)
{
	for (const EntityPtr& entity : mEntities)
	{
		if (entity->isDynamicsEnabled())
		{
			entity->updatePreDynamicsSubstep(dtSubstep);

			// Apply gravity
			auto position = getPosition(*entity);
			auto body = entity->getFirstComponent<DynamicBodyComponent>();
			if (body)
			{
				Vector3 force = mWorld->calcGravity(*position, body->getMass());
				body->applyCentralForce(force);
			}
		}
	}
}

void EntitySystem::updateDynamicsSubstep(double dtSubstep)
{
	for (const EntityPtr& entity : mEntities)
	{
		if (entity->isDynamicsEnabled())
		{
			entity->updateDynamicsSubstep(dtSubstep);
		}
	}
}

void EntitySystem::updatePostDynamicsSubstep(double dtSubstep)
{
	for (const EntityPtr& entity : mEntities)
	{
		if (entity->isDynamicsEnabled())
		{
			entity->updatePostDynamicsSubstep(dtSubstep);
		}
	}
}

void EntitySystem::updatePostDynamics(const System::StepArgs& args)
{
	for (const EntityPtr& entity : mEntities)
	{
		if (entity->isDynamicsEnabled())
		{
			entity->updatePostDynamics(args.dtSim, args.dtWallClock);
		}
	}
	for (const EntityPtr& entity : mEntities)
	{
		entity->updateAttachments(args.dtSim, args.dtWallClock);
	}
	mEntities.clear(); // clear container so that ownership is not held
}

} // namespace sim
} // namespace skybolt