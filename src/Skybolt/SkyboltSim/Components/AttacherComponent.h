/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include "SkyboltSim/Serialization/Serialization.h"

#include <string>

namespace skybolt {
namespace sim {

struct AttachmentState
{
	EntityId parentEntityId = nullEntityId();
	std::string parentEntityAttachmentPoint; //!< If empty, attaches to parent entity transform
	std::string ownEntityAttachmentPoint; //!< If empty, attaches to own entity transform
	sim::Vector3 positionOffset = math::dvec3Zero(); //!< Position offset from parent attachment point to own attachment point, in the frame of the parent attachment point
	sim::Quaternion orientationOffset = math::dquatIdentity(); //!< Orientation offset from parent attachment point to own attachment point
};

//! Component that attaches own entity to another entity
class AttacherComponent : public Component, public ExplicitSerialization
{
public:
	AttacherComponent(const World* world, Entity* ownEntity);
	~AttacherComponent();

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::Attachments, updatePose)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	std::optional<AttachmentState> state;

public: // ExplicitSerialization interface
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const override;
	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j) override;

private:
	void updatePose();

	//! @return true on success
	bool calcParentAttachmentPointPose(const Entity& parentEntity, const AttachmentState& attachmentState, sim::Vector3& position, sim::Quaternion& orientation) const;

private:
	const World* mWorld;
	Entity* mOwnEntity;
};

SKYBOLT_REFLECT_EXTERN(AttachmentState)
SKYBOLT_REFLECT_EXTERN(AttacherComponent)

} // namespace sim
} // namespace skybolt