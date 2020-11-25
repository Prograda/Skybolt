/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>

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

	float pitchNeutralMoment;
	float pitchDueToAngleOfAttack;
	float pitchDueToPitchRate;

	float yawDueToSideSlipAngle;
	float yawDueToRollRate;
	float yawDueToYawRate;
};

class FuselageComponent : public Component
{
public:
	FuselageComponent(const FuselageParams& params, Node* node, DynamicBodyComponent* body);

	void updatePreDynamicsSubstep(TimeReal dt);

	float getAngleOfAttack() const { return mAngleOfAttack; }
	float getSideSlipAngle() const { return mSideSlipAngle; }

private:
	Vector3 calcDragForce(const Vector3 &velocityLocal, const Vector3 &dragDirection, float density) const;
	Vector3 calcMoment(const Vector3 &angularVelocity, float angleOfAttackFactor, float sideSlipFactor,
				   float velSqLength, float airDensity) const;

private:
	const FuselageParams mParams;
	Node* mNode;
	DynamicBodyComponent* mBody;

	float mAngleOfAttack = 0;
	float mSideSlipAngle = 0;
};

} // namespace sim
} // namespace skybolt