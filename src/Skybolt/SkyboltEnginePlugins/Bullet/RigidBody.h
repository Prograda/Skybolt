/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"
#include <btBulletDynamicsCommon.h>

namespace skybolt {
namespace sim {

class RigidBody : public btRigidBody
{
public:
	RigidBody(btDiscreteDynamicsWorld* world, btCollisionShape*, int collisionGroupMask, int collisionFilterMask,
			  double mass, const btVector3 &inertia, const btVector3 &position, const btQuaternion &orientation, const btVector3 &velocity);
	~RigidBody();

	void setKinematic(bool kinematic);

	btVector3 getPosition() const;
	btQuaternion getOrientation() const;

	void setPosition(const btVector3&);
	void setOrientation(const btQuaternion&);

	void setCollisionGroupMask(int mask);
	int getCollisionGroupMask() const {return mCollisionGroupMask;}

private:
	btDiscreteDynamicsWorld* mWorld;
	int mCollisionFilterMask;
	int mCollisionGroupMask;
	bool mInWorld;
};

} // namespace sim
} // namespace skybolt