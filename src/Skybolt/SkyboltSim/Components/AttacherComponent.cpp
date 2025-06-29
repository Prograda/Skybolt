/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttacherComponent.h"
#include "AttachmentPointsComponent.h"
#include "Motion.h"
#include "NameComponent.h"
#include "SkyboltSim/World.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(AttacherComponent)
{
	registry.type<AttacherComponent>("AttacherComponent")
		.superType<Component>()
		.property("enabled", &AttacherComponent::enabled)
		.property("parentEntity", &AttacherComponent::getParentEntityName, &AttacherComponent::setParentEntityByName)
		.property("parentEntityAttachmentPoint", &AttacherComponent::parentEntityAttachmentPoint)
		.property("ownEntityAttachmentPoint", &AttacherComponent::ownEntityAttachmentPoint)
		.property("positionOffset", &AttacherComponent::positionOffset)
		.property("orientationOffset", &AttacherComponent::orientationOffset);
}
SKYBOLT_REFLECT_END

AttacherComponent::AttacherComponent(const World* world, Entity* ownEntity) :
	mWorld(world),
	mOwnEntity(ownEntity)
{
	assert(mWorld);
	assert(mOwnEntity);
}

AttacherComponent::~AttacherComponent() = default;

std::string AttacherComponent::getParentEntityName() const
{
	if (parentEntityId == sim::nullEntityId()) { return std::string(); }

	EntityPtr entity = mWorld->getEntityById(parentEntityId);
	if (!entity) { return std::string(); }

	return getName(*entity);
}

void AttacherComponent::setParentEntityByName(const std::string& name)
{
	EntityPtr entity = mWorld->findObjectByName(name);
	if (!entity) { return; }

	parentEntityId = entity->getId();
}

void AttacherComponent::updatePose()
{
	if (!enabled || parentEntityId == sim::nullEntityId()) { return; }

	const EntityPtr& parentEntity = mWorld->getEntityById(parentEntityId);
	if (!parentEntity) { return; }

	// Calculate parent attachment point pose
	sim::Vector3 position;
	sim::Quaternion orientation;
	if (!calcParentAttachmentPointPose(*parentEntity, position, orientation)) { return; }

	// Apply position and orientation offset
	position += orientation * positionOffset;
	orientation = orientation * orientationOffset;

	// Apply own attachment point transform
	if (!ownEntityAttachmentPoint.empty())
	{
		auto ownAttachmentPoint = findAttachmentPoint(*mOwnEntity, ownEntityAttachmentPoint);
		if (!ownAttachmentPoint) { return; }
	
		orientation *= glm::inverse(ownAttachmentPoint->orientationRelBody * glm::angleAxis(math::piD(), glm::dvec3(0,1,0)));
		position -= orientation * ownAttachmentPoint->positionRelBody;
	}

	// Update own entity pose
	setPosition(*mOwnEntity, position);
	setOrientation(*mOwnEntity, orientation);

	// Update velocity
	if (auto ownMotion = mOwnEntity->getFirstComponent<Motion>(); ownMotion)
	{
		if (auto parentMotion = parentEntity->getFirstComponent<Motion>())
		{
			ownMotion->linearVelocity = parentMotion->linearVelocity;
			ownMotion->angularVelocity = parentMotion->angularVelocity;
		}
		else
		{
			ownMotion->linearVelocity = math::dvec3Zero();
			ownMotion->angularVelocity = math::dvec3Zero();
		}
	}
}

bool AttacherComponent::calcParentAttachmentPointPose(const Entity& parentEntity, sim::Vector3& position, sim::Quaternion& orientation) const
{
	if (parentEntityAttachmentPoint.empty())
	{
		// Attach to entity
		auto optionalPosition = getPosition(parentEntity);
		auto optionalOrientation = getOrientation(parentEntity);
		if (!optionalPosition || !optionalOrientation) { return false; }

		position = *optionalPosition;
		orientation = *optionalOrientation;
	}
	else
	{
		// Attach to attachment point
		auto parentAttachmentPoint = findAttachmentPoint(parentEntity, parentEntityAttachmentPoint);
		if (!parentAttachmentPoint) { return false; }

		position = calcAttachmentPointPosition(parentEntity, *parentAttachmentPoint);
		orientation = calcAttachmentPointOrientation(parentEntity, *parentAttachmentPoint);
	}
	return true;
}

} // namespace sim
} // namespace skybolt