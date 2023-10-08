/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SimpleDynamicBodyComponent.h"
#include "Motion.h"
#include "Node.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(SimpleDynamicBodyComponent)
{
	registry.type<SimpleDynamicBodyComponent>("SimpleDynamicBodyComponent")
		.superType<DynamicBodyComponent>();
}
SKYBOLT_REFLECT_END

SimpleDynamicBodyComponent::SimpleDynamicBodyComponent(Node* node, Motion* motion, double mass, const Vector3& momentofInertia) :
	mNode(node),
	mMotion(motion),
	mMass(mass),
	mMomentOfInertia(momentofInertia)
{
	assert(mNode);
	assert(mMotion);
}

void SimpleDynamicBodyComponent::applyCentralForce(const Vector3& force)
{
	mTotalForce += force;
	mCurrentForces.push_back(AppliedForce({math::dvec3Zero(), force}));
}

void SimpleDynamicBodyComponent::applyForce(const Vector3& force, const Vector3& relPosition)
{
	mTotalForce += force;
	mCurrentForces.push_back(AppliedForce({relPosition, force}));

	Vector3 offset = relPosition - mNode->getOrientation() * mCenterOfMass;
	mTotalTorque += cross(offset, force);
}

void SimpleDynamicBodyComponent::applyTorque(const Vector3& torque)
{
	mTotalTorque += torque;
}

void SimpleDynamicBodyComponent::advanceSimTime(SecondsD newTime, SecondsD dtSubstep)
{
	mElapsedDt += dtSubstep;
}

void SimpleDynamicBodyComponent::integrateTimeStep()
{
	SecondsD dt = 0;
	std::swap(dt, mElapsedDt);

	double halfDt2 = dt * dt * 0.5;

	// Integrate linear dynamics using velocity-verlet https://en.wikipedia.org/wiki/Verlet_integration
	if (mMass > 0)
	{
		Vector3 acceleration = mTotalForce / (double)mMass;
		mNode->setPosition(mNode->getPosition() + mMotion->linearVelocity * dt + acceleration * halfDt2);
		mMotion->linearVelocity += acceleration * dt;
	}

	// Integrate angular dynamics using velocity-verlet
	if (mMomentOfInertia != math::dvec3Zero())
	{
		Quaternion ori = mNode->getOrientation();
		Vector3 torqueBodySpace = inverse(ori) * mTotalTorque;
		Vector3 angularAcceleration = ori * (torqueBodySpace / mMomentOfInertia);

		// https://gafferongames.com/post/physics_in_3d/
		Vector3 angularDisplacement = mMotion->angularVelocity * dt + angularAcceleration * halfDt2;
		Quaternion angularDisplacementQuat(
			0,
			angularDisplacement.x, 
			angularDisplacement.y, 
			angularDisplacement.z);

		ori += 0.5 * angularDisplacementQuat * ori;
		ori = normalize(ori);
		mNode->setOrientation(ori);

		mMotion->angularVelocity += angularAcceleration * dt;
	}

	// Reset accumulated forces and torques
	mTotalForce = math::dvec3Zero();
	mTotalTorque = math::dvec3Zero();
	
	std::swap(mCurrentForces, mForces);
	mCurrentForces.clear();
}

} // namespace sim
} // namespace skybolt