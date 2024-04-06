/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "BulletDynamicBodyComponent.h"
#include "BulletWorld.h"
#include "BulletTypeConversion.h"
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/CollisionGroupMasks.h>

namespace skybolt {
namespace sim {

BulletDynamicBodyComponent::BulletDynamicBodyComponent(BulletWorld* world, Node* node, double mass, const btVector3 &momentOfInertia, btCollisionShape* shape,
	const btVector3 &velocity, int collisionGroupMask, int collisionFilterMask) :
	mMinSpeedForCcdSquared(5.0 * 5.0),
	mWorld(world),
	mNode(node),
	mMomentOfInertia(momentOfInertia),
	mMass(mass),
	mForceIntegrationEnabled(true)
{
	mBody = mWorld->createRigidBody(shape, mass, mMomentOfInertia, btVector3(0,0,0), btQuaternion::getIdentity(), velocity, collisionGroupMask, collisionFilterMask);
	mBody->setFriction(1.0);
	mBody->setDamping(0.0, 0.0);
	mBody->setUserPointer(this);

	setPosition(mNode->getPosition());
	setOrientation(mNode->getOrientation());
}

BulletDynamicBodyComponent::~BulletDynamicBodyComponent()
{
	mWorld->destroyRigidBody(mBody);
}

void BulletDynamicBodyComponent::updatePreDynamics()
{
	mForces.clear();
	// Reset position and orientation if the node was moved by an external source since the last timestep
	auto newNodePosition = mNode->getPosition();
	if (mNodePosition != newNodePosition)
	{
		setPosition(mNode->getPosition());
	}

	auto newNodeOrientation = mNode->getOrientation();
	if (mNodeOrientation != newNodeOrientation)
	{
		setOrientation(mNode->getOrientation());
	}
}

void BulletDynamicBodyComponent::updatePostDynamics()
{
	// Calculate new node position
	btVector3 worldSpaceCenterOfMass = quatRotate(mBody->getOrientation(), mCenterOfMass);
	Vector3 newNodePosition = toGlmDvec3(mBody->getPosition() - worldSpaceCenterOfMass);

	btVector3 velocity = mBody->getLinearVelocity();
	if (velocity.length2() > mMinSpeedForCcdSquared)
	{
		// cheap and simple CCD
		// Note - we're using this instead of Bullet CCD because Bullet doesn't generate impulse for CCD
		const RayTestResult &res = mWorld->testRay(mNodePosition, newNodePosition, CollisionGroupMasks::terrain);
		if (res.hit)
		{
			// Correct body's position
			btTransform t = mBody->getCenterOfMassTransform();
			btVector3 aabbMin, aabbMax;
			mBody->getCollisionShape()->getAabb(t, aabbMin, aabbMax);
			double radius = aabbMax.distance(aabbMin) * 0.5;

			newNodePosition = res.position;
			t.setOrigin(toBtVector3(res.position) + worldSpaceCenterOfMass + toBtVector3(res.normal) * radius);
			mBody->setWorldTransform(t);
			mBody->proceedToTransform(t);

			// Correct body's velocity
			mBody->setLinearVelocity(btVector3(0, 0, 0));
		}
	}

	// Update node position and orientation
	mNode->setPosition(newNodePosition);
	mNode->setOrientation(toGlmDquat(mBody->getOrientation()));
}

void BulletDynamicBodyComponent::setDynamicsEnabled(bool enabled)
{
	mForceIntegrationEnabled = enabled;
	if (enabled)
	{
		mBody->setMassProps(mMass, mMomentOfInertia);
	}
	else
	{
		mBody->setMassProps(0, mMomentOfInertia);
		mBody->setLinearVelocity(btVector3(0,0,0));
	}
}

void BulletDynamicBodyComponent::setLinearVelocity(const Vector3& v)
{
	mBody->setLinearVelocity(toBtVector3(v));
}

Vector3 BulletDynamicBodyComponent::getLinearVelocity() const
{
	return toGlmDvec3(mBody->getLinearVelocity());
}

void BulletDynamicBodyComponent::setAngularVelocity(const Vector3& v)
{
	mBody->setAngularVelocity(toBtVector3(v));
}

Vector3 BulletDynamicBodyComponent::getAngularVelocity() const
{
	return toGlmDvec3(mBody->getAngularVelocity());
}

void BulletDynamicBodyComponent::setPosition(const Vector3& position)
{
	mNodePosition = position;
	btVector3 worldSpaceCenterOfMass = quatRotate(mBody->getOrientation(), mCenterOfMass);
	mBody->setPosition(toBtVector3(position) + worldSpaceCenterOfMass);
}

void BulletDynamicBodyComponent::setOrientation(const Quaternion& orientation)
{
	mNodeOrientation = orientation;
	mBody->setOrientation(toBtQuaternion(orientation));
}

void BulletDynamicBodyComponent::applyCentralForce(const Vector3& force)
{
	mBody->applyCentralForce(toBtVector3(force));

	AppliedForce appliedForce;
	appliedForce.force = force;
	appliedForce.positionRelBody = toGlmDvec3(quatRotate(mBody->getOrientation(), mCenterOfMass));
	mForces.push_back(appliedForce);
}

void BulletDynamicBodyComponent::applyForce(const Vector3& force, const Vector3& relPosition)
{
	mBody->applyForce(toBtVector3(force), toBtVector3(relPosition) - quatRotate(mBody->getOrientation(), mCenterOfMass));

	AppliedForce appliedForce;
	appliedForce.force = force;
	appliedForce.positionRelBody = relPosition;
	mForces.push_back(appliedForce);
}

void BulletDynamicBodyComponent::applyTorque(const Vector3& torque)
{
	mBody->applyTorque(toBtVector3(torque));
}

void BulletDynamicBodyComponent::setCenterOfMass(const Vector3& relPosition)
{
	mCenterOfMass = toBtVector3(relPosition);
	setPosition(mNodePosition); // update rigid body position
}

void BulletDynamicBodyComponent::setMass(double mass)
{
	mMass = mass;
	if (mForceIntegrationEnabled)
	{
		mBody->setMassProps(mass, mMomentOfInertia);
	}
}

void BulletDynamicBodyComponent::setCollisionsEnabled(bool enabled)
{
	if (enabled)
		mBody->setCollisionGroupMask(~0);
	else
		mBody->setCollisionGroupMask(0);
}

} // namespace sim
} // namespace skybolt