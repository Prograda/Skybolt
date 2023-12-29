/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltBulletFwd.h"
#include <btBulletDynamicsCommon.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/System/CollisionSystem.h>
#include <memory>

namespace skybolt {
namespace sim {

class RigidBody;

typedef std::shared_ptr<btDiscreteDynamicsWorld> btDiscreteDynamicsWorldPtr;


class BulletWorld
{
public:
	BulletWorld();

	RigidBody* createRigidBody(const btCollisionShapePtr& shape, double mass, const btVector3 &inertia, const btVector3 &position,
		const btQuaternion &orientation = btQuaternion::getIdentity(), const btVector3 &velocity = btVector3(0, 0, 0),
		int collisionGroupMask = ~0, int collisionFilterMask = ~0);

	void destroyRigidBody(RigidBody* body);

	inline btDiscreteDynamicsWorld* getDynamicsWorld() { return mDynamicsWorld.get(); }

	virtual std::optional<RayIntersectionResult> intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask);

private:
	btDiscreteDynamicsWorldPtr mDynamicsWorld;
};

} // namespace sim
} // namespace skybolt
