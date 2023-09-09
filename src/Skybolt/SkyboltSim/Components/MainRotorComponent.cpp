/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "MainRotorComponent.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/Motion.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Units.h>

using namespace std;

namespace skybolt {
namespace sim {

MainRotorComponent::MainRotorComponent(const MainRotorComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mMotion(config.motion),
	mBody(config.body),
	mDriverRpm(0.0f),
	mPitch(config.params->minPitch),
	mDesiredPitch(config.params->minPitch),
	mRpm(0.0f),
	mRotationAngle(0.0f),
	mTppPitch(config.params->tppPitchOffset),
	mTppRoll(0.0f),
	mPositionRelBody(config.positionRelBody),
	mOrientationRelBody(config.orientationRelBody),
	mTppOriRelBody(config.orientationRelBody),
	mCyclicInput(config.cyclicInput),
	mCollectiveInput(config.collectiveInput)
{
	assert(mNode);
	assert(mMotion);
	assert(mBody);
	assert(mCyclicInput);
	assert(mCollectiveInput);
}

void MainRotorComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

void MainRotorComponent::updatePreDynamicsSubstep()
{
	SecondsD dt = 0;
	std::swap(mDt, dt);

	// Apply controls
	mTppPitch = mCyclicInput->value.y * mParams->maxTppPitch + mParams->tppPitchOffset;
	mTppRoll = mCyclicInput->value.x * mParams->maxTppRoll;
	mDesiredPitch = mCollectiveInput->value * mParams->pitchRange + mParams->minPitch;

	// 1st order lag function
	mPitch += (mDesiredPitch - mPitch) * mParams->pitchResponseRate * dt;

	// Calc rotor RPM
	assert(mDriverRpm >= 0.0f && mDriverRpm <= 1.0f);
	mRpm = mDriverRpm * mParams->maxRpm;

	mTppOriRelBody = mOrientationRelBody * glm::angleAxis((double)mTppPitch, Vector3(0, 1, 0)) * glm::angleAxis((double)mTppRoll, Vector3(1, 0, 0));

	// Calc lift
	Quaternion bodyOrientation = mNode->getOrientation();
	const Vector3 velocityLocal = glm::inverse(bodyOrientation) * mMotion->linearVelocity;
	float velSqLength = (float)glm::dot(velocityLocal, velocityLocal);

	float inducedVel = calculateInducedVelocity(velSqLength); // induced velocity curve lookup
	inducedVel *= mDriverRpm;

	const Vector3 rotorLift = calculateLift(inducedVel);
	//assert(glm::length(rotorLift) < 1e10); // make sure it hasn't blown up

	mBody->applyForce(bodyOrientation * rotorLift, bodyOrientation * mPositionRelBody);

	mRotationAngle += mRpm * skybolt::rpmToRadPerSec * dt;
	mRotationAngle = fmod(mRotationAngle, skybolt::math::twoPiF());
}

float MainRotorComponent::calculateInducedVelocity(float velSqLength) const
{
	const std::vector<Vector3>& inducedVCurve = mParams->inducedVCurve;

	// Lookup induced velocity curve
	size_t numItems = inducedVCurve.size();
	if (numItems > 1)
	{
		for (size_t i = 1; i<numItems; i++)
		{
			if (velSqLength < inducedVCurve[i].y )
			{
				float interpFactor = (mParams->inducedVCurve[i].y - velSqLength) * inducedVCurve[i].z;
				return inducedVCurve[i-1].x * (interpFactor) + inducedVCurve[i].x * (1.0f-interpFactor);
			}
		}
	}
	else if (numItems > 0)
	{
		return inducedVCurve[0].y;
	}

	return 0.0f;
}

static const int elementCount = 4; //!< This is the number of integration elements to use around the rotor disk, not the number of rotor blades.
const Vector3 bladeDir[elementCount] = {Vector3(1,0,0), Vector3(0,1,0), Vector3(-1,0,0), Vector3(0,-1,0)};
float MainRotorComponent::calculateBladeElementLift(const Vector3& heliVelRelTpp, const Vector3& rotorVelRelTpp, int diskSector) const
{
	Vector3 velRelTpp = rotorVelRelTpp + heliVelRelTpp;

	float velRelBladeInTpp = fabs((float)glm::dot(Vector3(velRelTpp.x, velRelTpp.y, 0.0), bladeDir[diskSector]));
	if (velRelBladeInTpp == 0.0f)
		return 0.0f;

	float alpha = mPitch + (float)atan(velRelTpp.z / velRelBladeInTpp);

	const float airDensity = 1.225f; // TODO: pass in
	float averageBladePatchVelocitySq = 0.5f * velRelBladeInTpp * velRelBladeInTpp;
	float lift = mParams->liftConst * airDensity * (alpha - (mParams->zeroLiftAlpha)) * averageBladePatchVelocitySq;
	return lift;
}

Vector3 MainRotorComponent::calculateLift(float inducedVel) const
{
	//calculate relative velocity experienced by rotor tpp plane
	Vector3 tppFrameHeliVel = glm::inverse(mNode->getOrientation() * mTppOriRelBody) * mMotion->linearVelocity;
	tppFrameHeliVel += (double)inducedVel * Vector3(0, 0, -1); //add induced velocity to the TPP (induced vel is downward)
	float tppFrameRotorVel = mParams->diskRadius * mRpm * skybolt::rpmToRadPerSec;

	//calculate lift
	float lift = 0;
	for (int i=0; i < elementCount; i++)
	{
		lift += calculateBladeElementLift(tppFrameHeliVel, (double)tppFrameRotorVel * bladeDir[i], i);
	}
	lift /= (float)elementCount;

	return mTppOriRelBody * Vector3(0, 0, -lift); //sum all rotor disk lift forces

}

} // namespace sim
} // namespace skybolt