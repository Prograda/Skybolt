/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/System/CollisionSystem.h>

class btCollisionObject;

namespace skybolt::sim {

class BulletWorld;

class BulletSystem : public CollisionSystem
{
public:
	BulletSystem(BulletWorld* world);

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::DynamicsSubStep, performSubStep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	std::optional<RayIntersectionResult> intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask) const override;

	void performSubStep();

private:
	void processCollisionEvents();

private:
	BulletWorld* mWorld;
	double mDt = 0;
};

sim::EntityId getEntity(const btCollisionObject& object);

} // namespace skybolt::sim