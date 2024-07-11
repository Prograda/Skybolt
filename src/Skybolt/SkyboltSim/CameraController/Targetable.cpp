/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Targetable.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/NameComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(Targetable)
{
	registry.type<Targetable>("Targetable")
		.property("targetName", &Targetable::getTargetName, &Targetable::setTargetName);
}
SKYBOLT_REFLECT_END

Targetable::Targetable(World* world) :
	mWorld(world)
{
	assert(mWorld);
}

void Targetable::setTargetId(const EntityId& targetId)
{
	mTargetId = targetId;
}

Entity* Targetable::getTarget() const
{
	return mWorld->getEntityById(mTargetId).get();
}

const std::string& Targetable::getTargetName() const
{
	if (Entity* entity = getTarget(); entity)
	{
		return getName(*entity);
	}
	static const std::string empty = "";
	return empty;
}

void Targetable::setTargetName(const std::string& targetName)
{
	if (const EntityPtr& entity = mWorld->findObjectByName(targetName); entity)
	{
		mTargetId = entity->getId();
	}
	else
	{
		mTargetId = nullEntityId();
	}
}


} // namespace sim
} // namespace skybolt