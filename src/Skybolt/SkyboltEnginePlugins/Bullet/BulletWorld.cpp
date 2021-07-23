/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BulletWorld.h"
#include "RigidBody.h"
#include <SkyboltSim/CollisionGroupMasks.h>
#include "Bullet/BulletTypeConversion.h"

namespace skybolt {
namespace sim {

static btDiscreteDynamicsWorldPtr createDiscreteDynamicsWorld()
{
	auto broadphase = new btDbvtBroadphase();
	auto collisionConfiguration = new btDefaultCollisionConfiguration();
	auto dispatcher = new btCollisionDispatcher(collisionConfiguration);
	auto solver = new btSequentialImpulseConstraintSolver();

	btDiscreteDynamicsWorld* btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	btWorld->setGravity(btVector3(0, 0, 0));

	return btDiscreteDynamicsWorldPtr(btWorld, [=](btDiscreteDynamicsWorld* world) {
		delete world;
		delete solver;
		delete dispatcher;
		delete collisionConfiguration;
		delete broadphase;
	});
}

BulletWorld::BulletWorld() :
	mDynamicsWorld(createDiscreteDynamicsWorld())
{

}

RigidBody* BulletWorld::createRigidBody(btCollisionShape* shape, Real mass,  const btVector3 &inertia, const btVector3 &position,
										const btQuaternion &orientation, const btVector3 &velocity, int collisionGroupMask, int collisionFilterMask)
{
	RigidBody* body = new RigidBody(mDynamicsWorld.get(), shape, collisionGroupMask, collisionFilterMask, mass, inertia, position, orientation, velocity);
	if (mass == 0.0)
		body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
	return body;
}

void BulletWorld::destroyRigidBody(RigidBody* body)
{
	delete body; 
}

RayTestResult BulletWorld::testRay(const Vector3 &position, const Vector3 &direction, double length, int collisionFilterMask)
{
	Vector3 end = position + length * direction;
	return testRay(position, end, collisionFilterMask);
}

RayTestResult BulletWorld::testRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask)
{
	btVector3 startBullet = toBtVector3(start);
	btVector3 endBullet = toBtVector3(end);
	btCollisionWorld::ClosestRayResultCallback rayCallback(startBullet, endBullet);
	rayCallback.m_collisionFilterGroup = ~CollisionGroupMasks::terrain;
	rayCallback.m_collisionFilterMask = collisionFilterMask;

	if (startBullet.distance2(endBullet) > 1e-7)
	{
		mDynamicsWorld->rayTest(startBullet, endBullet, rayCallback);
	}

	RayTestResult result;
	result.hit = rayCallback.hasHit();
	if(result.hit)
	{
		result.position = toGlmDvec3(rayCallback.m_hitPointWorld);
		result.normal = toGlmDvec3(rayCallback.m_hitNormalWorld);
		result.distance = startBullet.distance(rayCallback.m_hitPointWorld);
	}
	return result;
}

} // namespace sim
} // namespace skybolt
