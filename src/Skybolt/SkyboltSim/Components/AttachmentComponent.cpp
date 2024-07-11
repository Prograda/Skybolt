/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachmentComponent.h"
#include "Motion.h"
#include "SkyboltSim/World.h"

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(AttachmentComponent)
{
	registry.type<AttachmentComponent>("AttachmentComponent")
		.superType<Component>()
		.property("parentEntityId", &AttachmentComponent::getParentEntityId, &AttachmentComponent::setParentEntityId);
}
SKYBOLT_REFLECT_END

AttachmentComponent::AttachmentComponent(const AttachmentParams& params, const World* world, Entity* childEntity) :
	mParams(params),
	mWorld(world),
	mChildEntity(childEntity)
{
	assert(mWorld);
	assert(mChildEntity);
}

AttachmentComponent::~AttachmentComponent() = default;

void AttachmentComponent::setParentEntityId(const EntityId& entityId)
{
	mParentEntityId = entityId;
	setTargetStateToParent();
}

void AttachmentComponent::setTargetStateToParent()
{
	if (mParentEntityId == nullEntityId())
	{
		return;
	}

	if (const EntityPtr& parentEntity = mWorld->getEntityById(mParentEntityId); parentEntity)
	{
		// Update position and orientation
		auto optionalPosition = getPosition(*parentEntity);
		auto optionalOrientation = getOrientation(*parentEntity);
		if (optionalPosition && optionalOrientation)
		{
			setPosition(*mChildEntity, *optionalPosition + *optionalOrientation * mParams.positionRelBody);
			setOrientation(*mChildEntity, *optionalOrientation * mParams.orientationRelBody);
		}

		// Update velocity
		if (auto childMotion = mChildEntity->getFirstComponent<Motion>(); childMotion)
		{
			if (auto parentMotion = parentEntity->getFirstComponent<Motion>())
			{
				childMotion->linearVelocity = parentMotion->linearVelocity;
				childMotion->angularVelocity = parentMotion->angularVelocity;
			}
		}
	}
}

} // namespace sim
} // namespace skybolt