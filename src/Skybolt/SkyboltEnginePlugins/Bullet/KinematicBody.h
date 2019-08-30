/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>

class btCollisionShape;

namespace skybolt {
namespace sim {

class BulletWorld;
class RigidBody;

class KinematicBody : public Component
{
public:
	KinematicBody(BulletWorld* world, Node* node, btCollisionShape* shape, int collisionGroupMask,
		 const Vector3 &localPosition = Vector3(0,0,0), const Quaternion &localOrientation = Quaternion(0,0,0,1));

	~KinematicBody();

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) override;

private:
	BulletWorld* mWorld;
	Node* mNode;
	RigidBody* mBody;
};

} // namespace sim
} // namespace skybolt
