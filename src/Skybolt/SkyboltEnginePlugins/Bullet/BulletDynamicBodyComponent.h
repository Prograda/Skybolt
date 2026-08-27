/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "BulletCollisionObject.h"
#include "RigidBody.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include <vector>
#include <map>
#include <set>

class btTypedConstraint;
class btGeneric6DofConstraint;

namespace skybolt {
namespace sim {

class BulletWorld;

struct BulletDynamicBodyComponentConfig
{
	BulletWorld* world;
	EntityId ownerEntityId;
	Node* node;
	Motion* motion;
	double mass;
	btVector3 momentOfInertia;
	btCollisionShapePtr shape;
	btVector3 velocity;
	int collisionGroupMask;
	int collisionFilterMask;
};

class BulletDynamicBodyComponent : public DynamicBodyComponent, public BulletCollisionObject
{
public: // DynamicBodyComponent interface
	BulletDynamicBodyComponent(const BulletDynamicBodyComponentConfig& config);
	~BulletDynamicBodyComponent() override;

	void setDynamicsEnabled(bool enabled) override;

	void setMass(double mass) override;
	double getMass() const override { return mMass; }

	Vector3 getMomentOfInertia() const override;
	void setMomentOfInertia(const Vector3& momentOfInertia) override;

	Vector3 getCenterOfMass() const override;

	void setCenterOfMass(const Vector3& relPosition) override;

	//! Apply force at center of mass. Force is in world axes.
	void applyCentralForce(const Vector3& force) override;

	//! Force and relPosition are in world axes
	void applyForce(const Vector3& force, const Vector3& relPosition) override;

	//! Apply torque. Torque is in world axes.
	void applyTorque(const Vector3& torque) override;


	void setLinearDamping(double damping) override;

	void setAngularDamping(double damping) override;

	RigidBody* getRigidBody() const { return mBody; }

public: // CollisionBody interface
	void setCollisionsEnabled(bool enabled) override;

	void setCollisionGroupMask(int mask) override;
	int getCollisionGroupMask() const override;

	void setCollisionFilterMask(int mask) override;
	int getCollisionFilterMask() const override;

public: // BulletCollisionObject interface
	const EntityId& getOwnerEntityId() const { return mOwnerEntityId; }

	const btCollisionObject* getBtCollisionObject() const override { return mBody; }

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(DynamicBodyComponent), typeid(CollisionBody), typeid(BulletDynamicBodyComponent), typeid(BulletCollisionObject)};
	}

public: // SimUpdatable interface
	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::PreDynamicsSubStep, updatePreDynamics)
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::PostDynamicsSubStep, updatePostDynamics)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamics();
	void updatePostDynamics();

protected:
	void setPosition(const Vector3& position);
	void setOrientation(const Quaternion& orientation);

protected:
	RigidBody* mBody;

private:
	const double mMinSpeedForCcdSquared;
	BulletWorld* mWorld;
	EntityId mOwnerEntityId;
	Node* mNode;
	Motion* mMotion;
	btVector3 mMomentOfInertia;
	Vector3 mNodePosition;
	Quaternion mNodeOrientation;
	btVector3 mCenterOfMass = btVector3(0,0,0); //!< Position of RigidBody relative to Node component in body axes.
	double mMass;

	bool mDynamicsEnabled;
	std::vector<AppliedForce> mCurrentForces; //!< For visualization purposes. Used to populate mForcesAppliedInLastSubstep in base class.
};

SKYBOLT_REFLECT_EXTERN(BulletDynamicBodyComponent);

} // namespace sim
} // namespace skybolt