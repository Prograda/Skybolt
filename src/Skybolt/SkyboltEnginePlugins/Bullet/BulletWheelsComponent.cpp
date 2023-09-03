/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BulletWheelsComponent.h"
#include "BulletTypeConversion.h"
#include <SkyboltSim/CollisionGroupMasks.h>

#include <btBulletDynamicsCommon.h>

namespace skybolt {
namespace sim {

// Same as ClosestRayResultCallback, but excludes a given object
struct  ClosestRayResultCallbackWithExclude : public btCollisionWorld::ClosestRayResultCallback
{
	ClosestRayResultCallbackWithExclude(const btVector3& from, const btVector3& to, btBroadphaseProxy* ignoreObject) :
		btCollisionWorld::ClosestRayResultCallback(from, to),
		mIgnoreObject(ignoreObject)
	{
		assert(ignoreObject);
	}

	~ClosestRayResultCallbackWithExclude() override = default;

	bool needsCollision(btBroadphaseProxy* proxy0) const override
	{
		if (proxy0 == mIgnoreObject)
		{
			return false;
		}
		return btCollisionWorld::ClosestRayResultCallback::needsCollision(proxy0);
	}

private:
	btBroadphaseProxy* mIgnoreObject;
};

// Overrides the ray to be excluded from the CollisionGroupMasks::terrain group, so that it can collide with terrain
// (since terrain can't collide with terrain).
class CustomVehicleRacaster : public btVehicleRaycaster
{
public:
	CustomVehicleRacaster(btDynamicsWorld* world, btBroadphaseProxy* ignoreObject) :
		m_dynamicsWorld(world),
		mIgnoreObject(ignoreObject)
	{
		assert(mIgnoreObject);
	}

	void* castRay(const btVector3& from, const btVector3& to, btVehicleRaycasterResult& result) override
	{
		ClosestRayResultCallbackWithExclude rayCallback(from, to, mIgnoreObject);
		rayCallback.m_collisionFilterGroup = ~CollisionGroupMasks::terrain;

		m_dynamicsWorld->rayTest(from, to, rayCallback);

		if (rayCallback.hasHit())
		{

			const btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObject);
			if (body && body->hasContactResponse())
			{
				result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
				result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
				result.m_hitNormalInWorld.normalize();
				result.m_distFraction = rayCallback.m_closestHitFraction;
				return (void*)body;
			}
		}
		return 0;
	}

private:
	btDynamicsWorld* m_dynamicsWorld;
	btBroadphaseProxy* mIgnoreObject;
};

constexpr double metersToCentimeters = 100.0;

BulletWheelsComponent::BulletWheelsComponent(const BulletWheelsComponentConfig& config) :
	mWorld(config.world),
	mVehicleRayCaster(std::make_unique<CustomVehicleRacaster>(config.world, config.body->getBroadphaseProxy())),
	mSteering(config.steering)
{
	btRaycastVehicle::btVehicleTuning tuning;
	mVehicle = std::make_unique<btRaycastVehicle>(tuning, config.body, mVehicleRayCaster.get());
	mVehicle->setCoordinateSystem(1, 2, 0);

	int i = 0;
	for (const auto& wheel : config.wheels)
	{
		tuning.m_maxSuspensionForce = config.mass * 1000;
		tuning.m_maxSuspensionTravelCm = wheel.maxSuspensionTravelLength * metersToCentimeters;
		tuning.m_suspensionStiffness = wheel.stiffness;
		tuning.m_suspensionDamping = wheel.dampingRelaxation * 2.0 * btSqrt(wheel.stiffness); // daming conversion from http://blender3d.org.ua/forum/game/iwe/upload/Vehicle_Simulation_With_Bullet.pdf
		tuning.m_suspensionCompression = wheel.dampingCompression * 2.0 * btSqrt(wheel.stiffness);
		tuning.m_frictionSlip = 1.0;

		double suspensionPaddingLength = 1.0; // padding added to avoid vehicle falling through ground if falling quickly
		double suspensionRestLength = wheel.maxSuspensionTravelLength + suspensionPaddingLength;
		bool isFrontWheel = false; // does not appear to be used by bullet
		mVehicle->addWheel(toBtVector3(wheel.attachmentPoint) + btVector3(0, 0, -suspensionPaddingLength), btVector3(0, 0, 1), btVector3(0, 1, 0), suspensionRestLength, wheel.wheelRadius, tuning, isFrontWheel);

		mMaxSteeringAnglesRadians.push_back(wheel.maxSteeringAngleRadians);
		mDrivingWheels.push_back(i);
		++i;
	}
	mWorld->addAction(mVehicle.get());
}

BulletWheelsComponent::~BulletWheelsComponent()
{
	mWorld->removeAction(mVehicle.get());
}

void BulletWheelsComponent::updatePreDynamics()
{
	if (mSteering)
	{
		double steering = -glm::sign(mSteering->value) * std::pow(std::abs(mSteering->value), 3.0);

		for (int i = 0; i < mVehicle->getNumWheels(); ++i)
		{
			mVehicle->setSteeringValue(steering * mMaxSteeringAnglesRadians[i], i);
		}
	}
}

void BulletWheelsComponent::setDrivingForce(double force)
{
	for (int i : mDrivingWheels)
	{
		mVehicle->applyEngineForce(force / double(mDrivingWheels.size()), i);
	}
}

} // namespace sim
} // namespace skybolt