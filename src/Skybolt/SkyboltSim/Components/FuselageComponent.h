/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include "SkyboltSim/Components/ControlInputsComponent.h"

namespace skybolt {
namespace sim {

struct FuselageParams
{
	// Lift
	float liftSlope; // Cl per radian
	float zeroLiftAlpha;
	float stallAlpha;
	float stallLift; // Cl in stalled region
	float liftArea; // m^2

	Vector3 dragConst; // Cd * area in each direction

	// Moments
	float momentMultiplier;

	float rollDueToSideSlipAngle;
	float rollDueToRollRate;
	float rollDueToYawRate;
	float rollDueToAileron;

	float pitchNeutralMoment;
	float pitchDueToAngleOfAttack;
	float pitchDueToPitchRate;
	float pitchDueToElevator;

	float yawDueToSideSlipAngle;
	float yawDueToRollRate;
	float yawDueToYawRate;
	float yawDueToRudder;

	std::optional<float> maxAutoTrimAngleOfAttack; //!< pitch auto trim disabled if empty
};

struct FuselageComponentConfig
{
	FuselageParams params;
	Node* node;
	Motion* motion;
	DynamicBodyComponent* body;
	ControlInputVec2Ptr stickInput; //!< Optional. Range is [-1, 1]. Positive backward and right.
	ControlInputFloatPtr rudderInput; //!< Optional. Range [-1, 1]
};

class FuselageComponent : public Component
{
public:
	FuselageComponent(const FuselageComponentConfig& config);

	float getAngleOfAttack() const { return mAngleOfAttack; }
	float getSideSlipAngle() const { return mSideSlipAngle; }

public: // SimUpdatable interface
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PreDynamicsSubStep, updatePreDynamicsSubstep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamicsSubstep();

private:
	Vector3 calcDragForce(const Vector3 &velocityLocal, const Vector3 &dragDirection, float density) const;
	Vector3 calcMoment(const Vector3 &angularVelocity, float angleOfAttackFactor, float sideSlipFactor,
				   float velSqLength, float airDensity) const;

	float calcTrimmedAngleOfAttack(float angleOfAttack, float airDensity, float speedSquared) const;

private:
	const FuselageParams mParams;
	Node* mNode;
	Motion* mMotion;
	DynamicBodyComponent* mBody;
	ControlInputVec2Ptr mStickInput;
	ControlInputFloatPtr mRudderInput;

	float mAngleOfAttack = 0;
	float mSideSlipAngle = 0;
	SecondsD mDt = 0;
};

} // namespace sim
} // namespace skybolt