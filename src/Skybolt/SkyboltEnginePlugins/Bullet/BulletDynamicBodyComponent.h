/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

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

class BulletDynamicBodyComponent : public DynamicBodyComponent
{
public:
	BulletDynamicBodyComponent(BulletWorld* world, Node* node, Real mass, const btVector3 &momentOfInertia, btCollisionShape* shape,
		const btVector3 &velocity, int collisionGroupMask, int collisionFilterMask);
	~BulletDynamicBodyComponent() override;

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) override;
	void updatePostDynamics(TimeReal dt, TimeReal dtWallClock) override;

	void setDynamicsEnabled(bool enabled) override;

	void setLinearVelocity(const Vector3& v) override;
	Vector3 getLinearVelocity() const override;

	//! Angular velocity is in world space
	void setAngularVelocity(const Vector3& v) override;
	Vector3 getAngularVelocity() const override;

	void setMass(Real mass) override;
	Real getMass() const override { return mMass; }

	void setCenterOfMass(const Vector3& relPosition) override;

	//! Apply force at center of mass. Force is in world axes.
	void applyCentralForce(const Vector3& force) override;

	//! Force and relPosition are in world axes
	void applyForce(const Vector3& force, const Vector3& relPosition) override;

	//! Apply torque. Torque is in world axes.
	void applyTorque(const Vector3& torque) override;

	void setCollisionsEnabled(bool enabled) override;

	RigidBody* getRigidBody() const { return mBody; }

public:
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(DynamicBodyComponent), typeid(BulletDynamicBodyComponent)};
	}

protected:
	void setPosition(const Vector3& position);
	void setOrientation(const Quaternion& orientation);

protected:
	RigidBody* mBody;

private:
	BulletWorld* mWorld;
	Node* mNode;
	btVector3 mMomentOfInertia;
	Vector3 mNodePosition;
	Quaternion mNodeOrientation;
	btVector3 mCenterOfMass = btVector3(0,0,0); //!< Position of RigidBody relative to Node component in body axes.
	Real mMass;


	float mTimeSinceCollided;
	float mMinSpeedForCcdSquared;
	bool mForceIntegrationEnabled;
};

} // namespace sim
} // namespace skybolt