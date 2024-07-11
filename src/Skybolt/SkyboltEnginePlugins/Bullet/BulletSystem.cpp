/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BulletSystem.h"
#include "BulletWorld.h"
#include "BulletDynamicBodyComponent.h"
#include "BulletTypeConversion.h"
#include "KinematicBody.h"

namespace skybolt::sim {

static EntityId getEntity(const Component& component)
{
	if (auto body = dynamic_cast<const BulletDynamicBodyComponent*>(&component); body)
	{
		return body->getOwnerEntityId();
	}
	else if (auto body = dynamic_cast<const KinematicBody*>(&component); body)
	{
		return body->getOwnerEntityId();
	}
	return nullEntityId();
}

sim::EntityId getEntity(const btCollisionObject& object)
{
	return getEntity(*static_cast<const Component*>(object.getUserPointer()));
}

BulletSystem::BulletSystem(BulletWorld* world) :
	mWorld(world)
{
	assert(mWorld);
}

void BulletSystem::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

std::optional<RayIntersectionResult> BulletSystem::intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask) const
{
	return mWorld->intersectRay(start, end, collisionFilterMask);
}

void BulletSystem::performSubStep()
{
	mWorld->getDynamicsWorld()->stepSimulation(mDt, 0, mDt);
	processCollisionEvents();
	mDt = 0;
};

void BulletSystem::processCollisionEvents()
{
	const auto& dynamicsWorld = mWorld->getDynamicsWorld();
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* objectA = contactManifold->getBody0();
		const btCollisionObject* objectB = contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.f)
			{
				CollisionEvent event;
				event.entityA = objectA->getUserPointer() ? getEntity(*objectA) : nullEntityId();
				event.entityB = objectB->getUserPointer() ? getEntity(*objectB) : nullEntityId();
				event.position = toGlmDvec3(pt.getPositionWorldOnB());
				event.normalB = toGlmDvec3(pt.m_normalWorldOnB);
				if (event.entityA != nullEntityId() || event.entityB != nullEntityId())
				{
					mEventEmitter->emitEvent(event);
				}
			}
		}
	}
}

} // namespace skybolt::sim