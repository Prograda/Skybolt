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

SKYBOLT_REFLECT_BEGIN(AttachmentState)
{
	registry.type<AttachmentState>("AttachmentState")
	.property("parentEntityAttachmentPoint", &AttachmentState::parentEntityAttachmentPoint)
	.property("ownEntityAttachmentPoint", &AttachmentState::ownEntityAttachmentPoint)
	.property("positionOffset", &AttachmentState::positionOffset)
	.property("orientationOffset", &AttachmentState::orientationOffset);
}
SKYBOLT_REFLECT_END

SKYBOLT_REFLECT_BEGIN(AttacherComponent)
{
	registry.type<AttacherComponent>("AttacherComponent")
		.superType<Component>()
		.property("state", &AttacherComponent::state)
		.superType<ExplicitSerialization>();
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

void AttacherComponent::updatePose()
{
	const EntityPtr& parentEntity = mWorld->getEntityById(state->parentEntityId);
	if (!parentEntity) { return; }

	if (!state) { return; }

	// Calculate parent attachment point pose
	sim::Vector3 position;
	sim::Quaternion orientation;
	if (!calcParentAttachmentPointPose(*parentEntity, *state, position, orientation)) { return; }

	// Apply position and orientation offset
	position += orientation * state->positionOffset;
	orientation = orientation * state->orientationOffset;

	// Apply own attachment point transform
	if (!state->ownEntityAttachmentPoint.empty())
	{
		auto ownAttachmentPoint = findAttachmentPoint(*mOwnEntity, state->ownEntityAttachmentPoint);
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

bool AttacherComponent::calcParentAttachmentPointPose(const Entity& parentEntity, const AttachmentState& attachmentState, sim::Vector3& position, sim::Quaternion& orientation) const
{
	if (state->parentEntityAttachmentPoint.empty())
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
		auto parentAttachmentPoint = findAttachmentPoint(parentEntity, state->parentEntityAttachmentPoint);
		if (!parentAttachmentPoint) { return false; }

		position = calcAttachmentPointPosition(parentEntity, *parentAttachmentPoint);
		orientation = calcAttachmentPointOrientation(parentEntity, *parentAttachmentPoint);
	}
	return true;
}

nlohmann::json AttacherComponent::toJson(refl::TypeRegistry& typeRegistry) const
{
	nlohmann::json json;
	if (!state)
	{
		return json;
	}
	
	auto entity = mWorld->getEntityById(state->parentEntityId);
	if (!entity)
	{
		return json;
	}

	if (std::string parentName = getName(*entity); !parentName.empty())
	{
		AttachmentState stateCopy = *state;
		json["state"] = writeReflectedObject(typeRegistry, refl::createNonOwningInstance(&typeRegistry, &stateCopy));
		json["state"]["parentEntity"] = parentName;
	}
	return json;
}

void AttacherComponent::fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
{
	state = std::nullopt;
	ifChildExists(j, "state", [&] (const nlohmann::json& stateJson) {
		state = AttachmentState{};
		std::string parentName = stateJson.at("parentEntity");
		if (auto entity = mWorld->findObjectByName(parentName); entity)
		{
			state->parentEntityId = entity->getId();
			refl::Instance instance = refl::createNonOwningInstance(&typeRegistry, &state.value());
			readReflectedObject(typeRegistry, instance, stateJson);
		}
	});
}

} // namespace sim
} // namespace skybolt