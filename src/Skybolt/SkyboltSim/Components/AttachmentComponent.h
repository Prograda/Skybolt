/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <string>

namespace skybolt {
namespace sim {

struct AttachmentParams
{
	Vector3 positionRelBody;
	Quaternion orientationRelBody;
};

//! Component that attaches a child entity to a parent entity
class AttachmentComponent : public Component
{
public:
	AttachmentComponent(const AttachmentParams& params, const World* world, Entity* childEntity);
	~AttachmentComponent();

	void setParentEntityId(const EntityId& target = nullEntityId());
	const EntityId& getParentEntityId() const { return mParentEntityId; }

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::Attachments, setTargetStateToParent)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void setPositionRelBody(const Vector3& positionRelBody) { mParams.positionRelBody = positionRelBody; }
	void setOrientationRelBody(const Quaternion& orientationRelBody) { mParams.orientationRelBody = orientationRelBody; }

private:
	void setTargetStateToParent();

private:
	AttachmentParams mParams;
	const World* mWorld;
	Entity* mChildEntity;
	EntityId mParentEntityId = nullEntityId();
};

SKYBOLT_REFLECT_EXTERN(AttachmentComponent)

} // namespace sim
} // namespace skybolt