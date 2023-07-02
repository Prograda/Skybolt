/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachmentComponent.h"
#include "DynamicBodyComponent.h"
#include "SkyboltSim/World.h"

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(AttachmentComponent)
{
	rttr::registration::class_<AttachmentComponent>("AttachmentComponent")
		.property("parentEntityId", &AttachmentComponent::getParentEntityId, &AttachmentComponent::setParentEntityId);
}

AttachmentComponent::AttachmentComponent(const AttachmentParams& params, const World* world, Entity* childEntity) :
	mParams(params),
	mWorld(world),
	mChildEntity(childEntity)
{
	assert(mWorld);
	assert(mParentObject);
}

AttachmentComponent::~AttachmentComponent() = default;

void AttachmentComponent::setParentEntityId(const EntityId& entityId)
{
	mParentEntityId = entityId;
	setTargetStateToParent();
}

void AttachmentComponent::updatePostDynamics(TimeReal dt, TimeReal dtWallClock)
{
	setTargetStateToParent();
}

void AttachmentComponent::setTargetStateToParent()
{
	if (mParentEntityId == nullEntityId())
	{
		return;
	}

	if (Entity* parentEntity = mWorld->getEntityById(mParentEntityId); parentEntity)
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
		if (auto childBody = mChildEntity->getFirstComponent<DynamicBodyComponent>(); childBody)
		{
			if (auto parentBody = parentEntity->getFirstComponent<DynamicBodyComponent>())
			{
				childBody->setLinearVelocity(parentBody->getLinearVelocity());
				childBody->setAngularVelocity(parentBody->getAngularVelocity());
			}
		}
	}
}

} // namespace sim
} // namespace skybolt