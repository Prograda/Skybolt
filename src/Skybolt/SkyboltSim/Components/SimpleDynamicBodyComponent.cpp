/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SimpleDynamicBodyComponent.h"
#include "Node.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

SimpleDynamicBodyComponent::SimpleDynamicBodyComponent(Node* node, Real mass, const Vector3& momentofInertia) :
	mNode(node),
	mMass(mass),
	mMomentOfInertia(momentofInertia)
{
}

void SimpleDynamicBodyComponent::applyCentralForce(const Vector3& force)
{
	mTotalForce += force;
}

void SimpleDynamicBodyComponent::applyForce(const Vector3& force, const Vector3& relPosition)
{
	mTotalForce += force;

	Vector3 offset = relPosition - mNode->getOrientation() * mCenterOfMass;
	mTotalTorque += cross(offset, force);
}

void SimpleDynamicBodyComponent::applyTorque(const Vector3& torque)
{
	mTotalTorque += torque;
}

void SimpleDynamicBodyComponent::updatePreDynamicsSubstep(TimeReal dtSubstep)
{
	double dt = (double)dtSubstep;
	double halfDt2 = dt * dt * 0.5;
	// integrate using velocity-verlet https://en.wikipedia.org/wiki/Verlet_integration
	if (mMass > 0)
	{
		Vector3 acceleration = mTotalForce / (double)mMass;
		mNode->setPosition(mNode->getPosition() + mLinearVelocity * dt + acceleration * halfDt2);
		mLinearVelocity += acceleration * dt;
	}

	if (mMomentOfInertia != math::dvec3Zero())
	{
		Quaternion ori = mNode->getOrientation();
		Vector3 torqueBodySpace = inverse(ori) * mTotalTorque;
		Vector3 angularAcceleration = ori * (torqueBodySpace / mMomentOfInertia);

		// https://gafferongames.com/post/physics_in_3d/
		Vector3 angularDisplacement = mAngularVelocity * dt + angularAcceleration * halfDt2;
		Quaternion angularDisplacementQuat(
			0,
			angularDisplacement.x, 
			angularDisplacement.y, 
			angularDisplacement.z);

		ori += 0.5 * angularDisplacementQuat * ori;
		ori = normalize(ori);
		mNode->setOrientation(ori);

		mAngularVelocity += angularAcceleration * dt;
	}
}

void SimpleDynamicBodyComponent::updatePostDynamics(TimeReal dt, TimeReal dtWallClock)
{
	mTotalForce = math::dvec3Zero();
	mTotalTorque = math::dvec3Zero();
}

} // namespace sim
} // namespace skybolt