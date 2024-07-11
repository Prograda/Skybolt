/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BulletWorld.h"
#include "RigidBody.h"
#include "BulletSystem.h"
#include "BulletTypeConversion.h"

#include <SkyboltSim/CollisionGroupMasks.h>

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
	btWorld->getSolverInfo().m_splitImpulse = false; // Disable because it allows objects to penetrate to far into the ground

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

RigidBody* BulletWorld::createRigidBody(const btCollisionShapePtr& shape, double mass,  const btVector3 &inertia, const btVector3 &position,
										const btQuaternion &orientation, const btVector3 &velocity, int collisionGroupMask, int collisionFilterMask)
{
	RigidBody* body = new RigidBody(mDynamicsWorld.get(), shape, collisionGroupMask, collisionFilterMask, mass, inertia, position, orientation, velocity);
	if (mass == 0.0)
	{
		body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
	}
	return body;
}

void BulletWorld::destroyRigidBody(RigidBody* body)
{
	delete body; 
}

std::optional<RayIntersectionResult> BulletWorld::intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask)
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

	if(rayCallback.hasHit())
	{
		RayIntersectionResult result;
		result.position = toGlmDvec3(rayCallback.m_hitPointWorld);
		result.normal = toGlmDvec3(rayCallback.m_hitNormalWorld);
		result.distance = startBullet.distance(rayCallback.m_hitPointWorld);
		result.entity = rayCallback.m_collisionObject ? getEntity(*rayCallback.m_collisionObject) : nullEntityId();
		return result;
	}
	return std::nullopt;
}

} // namespace sim
} // namespace skybolt
