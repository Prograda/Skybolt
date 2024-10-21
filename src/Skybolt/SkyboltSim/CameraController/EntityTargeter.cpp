/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityTargeter.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/NameComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(EntityTargeter)
{
	registry.type<EntityTargeter>("EntityTargeter")
		.property("targetName", &EntityTargeter::getTargetName, &EntityTargeter::setTargetName);
}
SKYBOLT_REFLECT_END

EntityTargeter::EntityTargeter(World* world) :
	mWorld(world)
{
	assert(mWorld);
}

const EntityId& EntityTargeter::getTargetId() const
{
	if (const EntityPtr& entity = mWorld->findObjectByName(mTargetName); entity)
	{
		return entity->getId();
	}
	static EntityId nullId = nullEntityId();
	return nullId;
}

void EntityTargeter::setTargetId(const EntityId& targetId)
{
	if (const EntityPtr& entity = mWorld->getEntityById(targetId); entity)
	{
		mTargetName = getName(*entity);
	}
}

Entity* EntityTargeter::getTarget() const
{
	return mWorld->findObjectByName(mTargetName).get();
}

const std::string& EntityTargeter::getTargetName() const
{
	return mTargetName;
}

void EntityTargeter::setTargetName(const std::string& targetName)
{
	mTargetName = targetName;
}


} // namespace sim
} // namespace skybolt