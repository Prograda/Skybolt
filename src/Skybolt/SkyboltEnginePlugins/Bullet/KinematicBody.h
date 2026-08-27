/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BulletCollisionObject.h"
#include "SkyboltBulletFwd.h"
#include <SkyboltSim/Component.h>
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Components/CollisionBody.h>

class btCollisionShape;

namespace skybolt {
namespace sim {

class BulletWorld;
class RigidBody;

class KinematicBody : public Component, public BulletCollisionObject, public CollisionBody
{
public:
	KinematicBody(BulletWorld* world, EntityId ownerEntityId, Node* node, const btCollisionShapePtr& shape, int collisionGroupMask,
		 const Vector3 &localPosition = Vector3(0,0,0), const Quaternion &localOrientation = Quaternion(0,0,0,1));

	~KinematicBody();

public: // CollisionBody interface
	void setCollisionsEnabled(bool enabled) override;
	void setCollisionGroupMask(int mask) override;
	int getCollisionGroupMask() const override;
	void setCollisionFilterMask(int mask) override;
	int getCollisionFilterMask() const override;

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(KinematicBody), typeid(BulletCollisionObject), typeid(CollisionBody)};
	}

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::PreDynamicsSubStep, updatePreDynamics)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamics();

public: // BulletCollisionObject interface
	
	const EntityId& getOwnerEntityId() const override { return mOwnerEntityId; }

	const btCollisionObject* getBtCollisionObject() const override;


private:
	BulletWorld* mWorld;
	EntityId mOwnerEntityId;
	Node* mNode;
	RigidBody* mBody;
};

SKYBOLT_REFLECT_EXTERN(KinematicBody);

} // namespace sim
} // namespace skybolt
