/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "RigidBody.h"

using namespace skybolt::sim;

RigidBody::RigidBody(btDiscreteDynamicsWorld* world, const btCollisionShapePtr& shape, int collisionGroupMask, int collisionFilterMask,
					  double mass, const btVector3 &inertia, const btVector3 &position, const btQuaternion &orientation, const btVector3 &velocity) :
	btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass,
		new btDefaultMotionState(btTransform(orientation, position)), shape.get(), inertia)),
	mWorld(world),
	mShape(shape),
	mCollisionFilterMask(collisionFilterMask),
	mInWorld(false)
{
	assert(mShape);

	setActivationState(DISABLE_DEACTIVATION);
	setDamping(0, 0);
	setLinearVelocity(velocity);
	setCollisionGroupMask(collisionGroupMask);
}

RigidBody::~RigidBody()
{
	mWorld->removeRigidBody(this);
	delete getMotionState();
}

void RigidBody::setKinematic(bool kinematic)
{
	if (kinematic)
		setCollisionFlags(getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	else
		setCollisionFlags(getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
}

btVector3 RigidBody::getPosition() const
{
	btTransform t;
	getMotionState()->getWorldTransform(t);
	return t.getOrigin();
}

btQuaternion RigidBody::getOrientation() const
{
	btTransform t;
	getMotionState()->getWorldTransform(t);
	return t.getRotation();
}

void RigidBody::setPosition(const btVector3& position)
{
	btTransform t;
	t = getWorldTransform();
	t.setOrigin(position);
	setCenterOfMassTransform(t);
	getMotionState()->setWorldTransform(t);
}

void RigidBody::setOrientation(const btQuaternion& orientation)
{
	btTransform t;
	getMotionState()->getWorldTransform(t);
	t.setRotation(orientation);
	setCenterOfMassTransform(t);
	getMotionState()->setWorldTransform(t);
}

void RigidBody::setCollisionGroupMask(int mask)
{
	mCollisionGroupMask = mask;
	if (mInWorld)
	{
		mWorld->removeRigidBody(this);
		mInWorld = false;
	}

	if (mask != 0)
	{
		mWorld->addRigidBody(this, mask, mCollisionFilterMask);
		mInWorld = true;
	}
}
