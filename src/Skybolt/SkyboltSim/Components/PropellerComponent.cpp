/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropellerComponent.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Units.h>

namespace skybolt {
namespace sim {

PropellerComponent::PropellerComponent(const PropellerComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mBody(config.body),
	mPositionRelBody(config.positionRelBody),
	mOrientationRelBody(config.orientationRelBody),
	mInput(config.input),
	mDriverRpm(0.0f),
	mPitch(config.pitch),
	mRotationAngle(0.0f),
	mRpm(0.0f)
{
	assert(mNode);
	assert(mBody);
}

void PropellerComponent::updatePreDynamicsSubstep(TimeReal dt)
{
	// Apply control input
	float desiredPitch = getUnitNormalized(*mInput) * mParams.pitchRange + mParams.minPitch;

	 // First order lag function of peddle input
	mPitch += desiredPitch - mPitch * std::min(1.0f, mParams.pitchResponseRate * dt);
	mRpm = mDriverRpm * mParams.rpmMultiplier;

	const float thrust = mRpm * mPitch * mParams.thrustPerRpmPerPitch;
	Vector3 forceRelBody = mOrientationRelBody * Vector3(0, 0, -thrust);

	Vector3 forcePos = mPositionRelBody;
	forcePos.z = 0; // Set to zero to eliminate undesired yaw-roll coupling. TODO: improve physics and remove this hack.

	Quaternion orientation = mNode->getOrientation();
	mBody->applyForce(orientation * forceRelBody, orientation * forcePos);

	mRotationAngle += mRpm * skybolt::rpmToRadPerSec * dt;
	mRotationAngle = fmod(mRotationAngle, skybolt::math::twoPiF());
}

} // namespace sim
} // namespace skybolt