/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FuselageComponent.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/Motion.h"
#include "SkyboltSim/Physics/Atmosphere.h"
#include "SkyboltSim/Spatial/GreatCircle.h"

#include <algorithm>

namespace skybolt {
namespace sim {

FuselageComponent::FuselageComponent(const FuselageComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mMotion(config.motion),
	mBody(config.body),
	mStickInput(config.stickInput),
	mRudderInput(config.rudderInput)
{
	assert(mNode);
	assert(mMotion);
	assert(mBody);
}

static double calcAltitude(const sim::Vector3& position)
{
	return glm::length(position) - earthRadius();
}

static double calcAirDensity(double altitude)
{
	static Atmosphere atmosphere = createEarthAtmosphere();
	return atmosphere.getDensity(altitude);
}

void FuselageComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

void FuselageComponent::updatePreDynamicsSubstep()
{
	SecondsD dt = 0;
	std::swap(mDt, dt);

	const Vector3& velocityLocal = glm::inverse(mNode->getOrientation()) * mMotion->linearVelocity;

	// angle of attack and side slip
	mAngleOfAttack = (float)std::atan2(velocityLocal.z, velocityLocal.x);
	mSideSlipAngle = (float)std::atan2(-velocityLocal.y, velocityLocal.x);

	// calculate lift
	Vector3 lift;
	const float airDensity = calcAirDensity(calcAltitude(mNode->getPosition()));

	float alphaDelta = mAngleOfAttack - mParams.zeroLiftAlpha;

	float liftCoeff;
	if (std::abs(alphaDelta) > mParams.stallAlpha)
	{
		liftCoeff = mParams.stallLift;
	}
	else
	{
		liftCoeff = mParams.liftSlope * alphaDelta;
	}

	lift = Vector3(0.0f, 0.0f, -liftCoeff * mParams.liftArea * 0.5f * airDensity * glm::dot(velocityLocal, velocityLocal));

	double speed = glm::length(mMotion->linearVelocity);
	
	//apply forces
	if (speed > 0.0f)
	{
		Vector3 drag = calcDragForce(velocityLocal, -mMotion->linearVelocity / speed, airDensity);
		mBody->applyCentralForce(drag);
	}

	Quaternion orientation = mNode->getOrientation();
	mBody->applyCentralForce(orientation * lift);

	// apply moments
	const float maxEffectiveSpeedForMoments = 100.0f;
	float speedForMoments = std::min(float(speed), maxEffectiveSpeedForMoments);
	float speedForMomentsSq = speedForMoments * speedForMoments;

	const Vector3 localAngularVelocity = glm::inverse(orientation) * mMotion->angularVelocity;

	double speedSquared = speed * speed;
	float trimmedAngleOfAttack = calcTrimmedAngleOfAttack(mAngleOfAttack, airDensity, speedSquared);

	const Vector3 moment = calcMoment(localAngularVelocity, sin(trimmedAngleOfAttack), sin(mSideSlipAngle), speedForMomentsSq, airDensity);

	mBody->applyTorque(orientation * moment * (double)mParams.momentMultiplier);
}

Vector3 FuselageComponent::calcDragForce(const Vector3 &velocityLocal, const Vector3 &dragDirection, float density) const
{
	double CDrag = velocityLocal.x*velocityLocal.x * mParams.dragConst.x
			     + velocityLocal.y*velocityLocal.y * mParams.dragConst.y
				 + velocityLocal.z*velocityLocal.z * mParams.dragConst.z;
	return dragDirection * CDrag * 0.5 * (double)density;
}

Vector3 FuselageComponent::calcMoment(const Vector3 &angularVelocity, float angleOfAttackFactor, float sideSlipFactor,
									  float velSqLength, float airDensity) const
{
	Vector3 moment;

	// Roll
	moment.x = mParams.rollDueToSideSlipAngle * sideSlipFactor * velSqLength + mParams.rollDueToRollRate * angularVelocity.x
		+ mParams.rollDueToYawRate * angularVelocity.z;
	
	// Pitch
	moment.y = mParams.pitchNeutralMoment * velSqLength + mParams.pitchDueToAngleOfAttack * angleOfAttackFactor * velSqLength
			 + mParams.pitchDueToPitchRate * angularVelocity.y;

	// Yaw
	moment.z = mParams.yawDueToSideSlipAngle * sideSlipFactor * velSqLength + mParams.yawDueToRollRate * angularVelocity.x + mParams.yawDueToYawRate * angularVelocity.z;

	// Control inputs
	if (mStickInput)
	{
		moment.x += mParams.rollDueToAileron * mStickInput->value.x * velSqLength;
		moment.y += mParams.pitchDueToElevator * mStickInput->value.y * velSqLength;
	}

	if (mRudderInput)
	{
		moment.z += mParams.yawDueToRudder * mRudderInput->value * velSqLength;
	}

	return moment * (double)airDensity;
}

float FuselageComponent::calcTrimmedAngleOfAttack(float angleOfAttack, float airDensity, float speedSquared) const
{
	if (mParams.maxAutoTrimAngleOfAttack)
	{
		float cruiseTrimAlpha = mBody->getMass() * 9.81f / (mParams.liftSlope * mParams.liftArea * 0.5f * airDensity * speedSquared);
		return mAngleOfAttack - std::clamp(cruiseTrimAlpha, -*mParams.maxAutoTrimAngleOfAttack, *mParams.maxAutoTrimAngleOfAttack);
	}
	else
	{
		return angleOfAttack;
	}
}

} // namespace sim
} // namespace skybolt