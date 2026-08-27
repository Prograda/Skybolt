/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "KinematicBody.h"
#include "BulletTypeConversion.h"
#include "BulletSystem.h"
#include "BulletWorld.h"
#include "RigidBody.h"
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/CollisionGroupMasks.h>
#include <SkyboltSim/Components/Node.h>

namespace skybolt::sim {

SKYBOLT_REFLECT(KinematicBody) {
	registry.type<KinematicBody>("KinematicBody")
		.superType<Component>();
}

KinematicBody::KinematicBody(BulletWorld* world, EntityId ownerEntityId, Node* node, const btCollisionShapePtr& shape, int collisionGroupMask,
					   const Vector3 &localPosition, const Quaternion &localOrientation) :
	mWorld(world),
	mOwnerEntityId(ownerEntityId),
	mNode(node)
{
	// TODO: un-hardcode collision filter mask
	mBody = world->createRigidBody(shape, 0, btVector3(0, 0, 0), toBtVector3(node->getPosition()), toBtQuaternion(node->getOrientation()), btVector3(0, 0, 0), collisionGroupMask, ~CollisionGroupMasks::terrain);
	mBody->setFriction(1.0);
	setPointerToWrappedCollisionObject(*mBody, this);
}

KinematicBody::~KinematicBody()
{
	mWorld->destroyRigidBody(mBody);
}

void KinematicBody::updatePreDynamics()
{
	mBody->setPosition(toBtVector3(mNode->getPosition()));
	mBody->setOrientation(toBtQuaternion(mNode->getOrientation()));
}

void KinematicBody::setCollisionsEnabled(bool enabled)
{
	if (enabled)
		mBody->setCollisionGroupMask(~0);
	else
		mBody->setCollisionGroupMask(0);
}

void KinematicBody::setCollisionGroupMask(int mask)
{
	mBody->setCollisionGroupMask(mask);
}

int KinematicBody::getCollisionGroupMask() const
{
	return mBody->getCollisionGroupMask();
}

void KinematicBody::setCollisionFilterMask(int mask)
{
	mBody->setCollisionFlags(mask);
}

int KinematicBody::getCollisionFilterMask() const
{
	return mBody->getCollisionFlags();
}

const btCollisionObject* KinematicBody::getBtCollisionObject() const
{
	return mBody;
}

} // namespace skybolt::sim