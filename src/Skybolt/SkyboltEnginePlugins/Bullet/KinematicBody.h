/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltBulletFwd.h"
#include <SkyboltSim/Component.h>
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SkyboltSimFwd.h>

class btCollisionShape;

namespace skybolt {
namespace sim {

class BulletWorld;
class RigidBody;

class KinematicBody : public Component
{
public:
	KinematicBody(BulletWorld* world, EntityId ownerEntityId, Node* node, const btCollisionShapePtr& shape, int collisionGroupMask,
		 const Vector3 &localPosition = Vector3(0,0,0), const Quaternion &localOrientation = Quaternion(0,0,0,1));

	~KinematicBody();

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::BeginStateUpdate, updatePreDynamics)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamics();

	EntityId getOwnerEntityId() const { return mOwnerEntityId; }

private:
	BulletWorld* mWorld;
	EntityId mOwnerEntityId;
	Node* mNode;
	RigidBody* mBody;
};

} // namespace sim
} // namespace skybolt
