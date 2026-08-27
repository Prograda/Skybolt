/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "BulletDynamicBodyComponent.h"
#include "BulletSystem.h"
#include "BulletWorld.h"
#include "BulletTypeConversion.h"
#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/CollisionGroupMasks.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(BulletDynamicBodyComponent) {
	registry.type<BulletDynamicBodyComponent>("BulletDynamicBodyComponent")
		.property("momentOfInertia", &BulletDynamicBodyComponent::getMomentOfInertia, &BulletDynamicBodyComponent::setMomentOfInertia)
		.property("centerOfMass", &BulletDynamicBodyComponent::getCenterOfMass, &BulletDynamicBodyComponent::setCenterOfMass)
		.superType<DynamicBodyComponent>();
}

constexpr std::size_t maxQueuedForces = 32;

BulletDynamicBodyComponent::BulletDynamicBodyComponent(const BulletDynamicBodyComponentConfig& config) :
	mMinSpeedForCcdSquared(5.0 * 5.0),
	mWorld(config.world),
	mOwnerEntityId(config.ownerEntityId),
	mNode(config.node),
	mMotion(config.motion),
	mMomentOfInertia(config.momentOfInertia),
	mMass(config.mass),
	mDynamicsEnabled(true)
{
	mBody = mWorld->createRigidBody(config.shape, config.mass, mMomentOfInertia, btVector3(0,0,0), btQuaternion::getIdentity(), config.velocity, config.collisionGroupMask, config.collisionFilterMask);
	mBody->setFriction(1.0);
	mBody->setDamping(0.0, 0.0);
	setPointerToWrappedCollisionObject(*mBody, this);

	setPosition(mNode->getPosition());
	setOrientation(mNode->getOrientation());
	mBody->setLinearVelocity(toBtVector3(mMotion->linearVelocity));
	mBody->setAngularVelocity(toBtVector3(mMotion->angularVelocity));
}

BulletDynamicBodyComponent::~BulletDynamicBodyComponent()
{
	mWorld->destroyRigidBody(mBody);
}

void BulletDynamicBodyComponent::updatePreDynamics()
{
	if (mDynamicsEnabled)
	{
		// Reset position and orientation if the node was moved by an external source since the last timestep
		auto newNodePosition = mNode->getPosition();
		if (mNodePosition != newNodePosition)
		{
			setPosition(newNodePosition);
		}

		auto newNodeOrientation = mNode->getOrientation();
		if (mNodeOrientation != newNodeOrientation)
		{
			setOrientation(newNodeOrientation);
		}

		mBody->setLinearVelocity(toBtVector3(mMotion->linearVelocity));
		mBody->setAngularVelocity(toBtVector3(mMotion->angularVelocity));
	}
}

void BulletDynamicBodyComponent::updatePostDynamics()
{
	if (mDynamicsEnabled)
	{
		// Calculate new node position
		btVector3 worldSpaceCenterOfMass = quatRotate(mBody->getOrientation(), mCenterOfMass);
		Vector3 newNodePosition = toGlmDvec3(mBody->getPosition() - worldSpaceCenterOfMass);

		btVector3 velocity = mBody->getLinearVelocity();
		if (velocity.length2() > mMinSpeedForCcdSquared)
		{
			// cheap and simple CCD
			// Note - we're using this instead of Bullet CCD because Bullet doesn't generate impulse for CCD
			if (auto result = mWorld->intersectRay(mNodePosition, newNodePosition, CollisionGroupMasks::terrain); result)
			{
				// Correct body's position
				btTransform t = mBody->getCenterOfMassTransform();
				btVector3 aabbMin, aabbMax;
				mBody->getCollisionShape()->getAabb(t, aabbMin, aabbMax);
				double radius = aabbMax.distance(aabbMin) * 0.5;

				newNodePosition = result->position;
				t.setOrigin(toBtVector3(result->position) + worldSpaceCenterOfMass + toBtVector3(result->normal) * radius);
				mBody->setWorldTransform(t);
				mBody->proceedToTransform(t);

				// Zero the body's velocity component in direction of penetration
				btVector3 velocity = mBody->getLinearVelocity();
				velocity -= toBtVector3(result->normal) * velocity.dot(toBtVector3(result->normal));
				mBody->setLinearVelocity(velocity);
			}
		}

		// Update node position and orientation
		mNodePosition = newNodePosition;
		mNodeOrientation = toGlmDquat(mBody->getOrientation());

		mNode->setPosition(mNodePosition);
		mNode->setOrientation(mNodeOrientation);
		mMotion->linearVelocity = toGlmDvec3(mBody->getLinearVelocity());
		mMotion->angularVelocity = toGlmDvec3(mBody->getAngularVelocity());
	}

	std::swap(mCurrentForces, mForcesAppliedInLastSubstep);
	mCurrentForces.clear();
}

Vector3 BulletDynamicBodyComponent::getMomentOfInertia() const
{
	return toGlmDvec3(mMomentOfInertia);
}

void BulletDynamicBodyComponent::setMomentOfInertia(const Vector3& momentOfInertia)
{
	mMomentOfInertia = toBtVector3(momentOfInertia);
	if (mDynamicsEnabled)
	{
		mBody->setMassProps(mMass, mMomentOfInertia);
	}
}

Vector3 BulletDynamicBodyComponent::getCenterOfMass() const
{
	return toGlmDvec3(mCenterOfMass);
}

void BulletDynamicBodyComponent::setCenterOfMass(const Vector3& relPosition)
{
	mCenterOfMass = toBtVector3(relPosition);
	setPosition(mNodePosition); // update rigid body position
}

void BulletDynamicBodyComponent::setMass(double mass)
{
	mMass = mass;
	if (mDynamicsEnabled)
	{
		mBody->setMassProps(mass, mMomentOfInertia);
	}
}

void BulletDynamicBodyComponent::setDynamicsEnabled(bool enabled)
{
	mDynamicsEnabled = enabled;
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

	if (mCurrentForces.size() < maxQueuedForces)
	{
		mCurrentForces.push_back(appliedForce);
	}
}

void BulletDynamicBodyComponent::applyForce(const Vector3& force, const Vector3& relPosition)
{
	mBody->applyForce(toBtVector3(force), toBtVector3(relPosition) - quatRotate(mBody->getOrientation(), mCenterOfMass));

	AppliedForce appliedForce;
	appliedForce.force = force;
	appliedForce.positionRelBody = relPosition;

	if (mCurrentForces.size() < maxQueuedForces)
	{
		mCurrentForces.push_back(appliedForce);
	}
}

void BulletDynamicBodyComponent::applyTorque(const Vector3& torque)
{
	mBody->applyTorque(toBtVector3(torque));
}

void BulletDynamicBodyComponent::setCollisionsEnabled(bool enabled)
{
	if (enabled)
		mBody->setCollisionGroupMask(~0);
	else
		mBody->setCollisionGroupMask(0);
}

void BulletDynamicBodyComponent::setLinearDamping(double damping)
{
	mBody->setDamping(damping, mBody->getAngularDamping());
}

void BulletDynamicBodyComponent::setAngularDamping(double damping)
{
	mBody->setDamping(mBody->getLinearDamping(), damping);
}

void BulletDynamicBodyComponent::setCollisionGroupMask(int mask)
{
	mBody->setCollisionGroupMask(mask);
}

int BulletDynamicBodyComponent::getCollisionGroupMask() const
{
	return mBody->getCollisionGroupMask();
}

void BulletDynamicBodyComponent::setCollisionFilterMask(int mask)
{
	mBody->setCollisionFilterMask(mask);
}

int BulletDynamicBodyComponent::getCollisionFilterMask() const
{
	return mBody->getCollisionFilterMask();
}

} // namespace sim
} // namespace skybolt