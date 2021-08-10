/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachmentComponent.h"
#include "DynamicBodyComponent.h"
#include "ParentReferenceComponent.h"

#include <assert.h>

namespace skybolt {
namespace sim {

AttachmentComponent::AttachmentComponent(const AttachmentParams& params, Entity* parentObject) :
	mParams(params),
	mParentObject(parentObject),
	mTarget(nullptr),
	mTargetListener(nullptr)
{
	assert(mParentObject);
}

AttachmentComponent::~AttachmentComponent()
{
	if (mTarget)
	{
		mTarget->removeListener(this);
	}
}

void AttachmentComponent::resetTarget(Entity* target)
{
	auto parentBody = mParentObject->getFirstComponent<DynamicBodyComponent>();

	// Remove previous target
	if (mTarget)
	{
		mTarget->removeListener(this);
		mTarget->removeComponent(mParentReference);
	}

	mTarget = target;

	// Add new target
	if (mTarget)
	{
		mParentReference = std::make_shared<ParentReferenceComponent>(mParentObject);
		mTarget->addComponent(mParentReference);
		mTarget->addListener(this);

		setTargetStateToParent();
	}
}

void AttachmentComponent::updatePostDynamics(TimeReal dt, TimeReal dtWallClock)
{
	setTargetStateToParent();
}

void AttachmentComponent::setTargetStateToParent()
{
	if (mTarget)
	{
		// Update position and orientation
		auto optionalPosition = getPosition(*mTarget);
		auto optionalOrientation = getOrientation(*mTarget);
		if (optionalPosition && optionalOrientation)
		{
			setPosition(*mParentObject, *optionalPosition + *optionalOrientation * mParams.positionRelBody);
			setOrientation(*mParentObject, *optionalOrientation * mParams.orientationRelBody);
		}

		// Update velocity
		auto targetBody = mTarget->getFirstComponent<DynamicBodyComponent>();
		if (targetBody)
		{
			auto parentBody = mParentObject->getFirstComponent<DynamicBodyComponent>();
			if (parentBody)
			{
				targetBody->setLinearVelocity(parentBody->getLinearVelocity());
				targetBody->setAngularVelocity(parentBody->getAngularVelocity());
			}
		}
	}
}

void AttachmentComponent::onDestroy(Entity* entity)
{
	resetTarget(nullptr);
}

AttachmentComponentPtr getParentAttachment(const sim::Entity& entity)
{
	if (auto parentReferenceComponent = entity.getFirstComponent<ParentReferenceComponent>())
	{
		auto parent = parentReferenceComponent->getParent();
		for (auto attachment : parent->getComponentsOfType<AttachmentComponent>())
		{
			if (attachment->getTarget() == &entity)
			{
				return attachment;
			}
		}
	}
	return nullptr;
}

} // namespace sim
} // namespace skybolt