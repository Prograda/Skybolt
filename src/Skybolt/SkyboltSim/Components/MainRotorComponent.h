/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <vector>

namespace skybolt {
namespace sim {

struct MainRotorParams
{
	float maxRpm;

	float pitchResponseRate;
	float minPitch;
	float pitchRange;
	float maxTppPitch; //!< relative to tppPitchOffset
	float maxTppRoll;
	float tppPitchOffset;

	float zeroLiftAlpha;
	float liftConst; //!< 0.5 * liftSlope[1/rad] * bladeSurfaceArea * bladeCount
	float diskRadius;

	//! x and y components store point on a curve of induced velocity vs flight speed squared. z component stores 1/(y2-y1)
	std::vector<Vector3> inducedVCurve;
};

typedef std::shared_ptr<MainRotorParams> MainRotorParamsPtr;

struct MainRotorComponentConfig
{
	MainRotorParamsPtr params;
	Node* node;
	Motion* motion;
	DynamicBodyComponent* body;
	Vector3 positionRelBody;
	Quaternion orientationRelBody;

	//! First component is positive right stick deflection, second component is positive backward stick deflection. Range is [-1, 1].
	ControlInputVec2Ptr cyclicInput;

	ControlInputFloatPtr collectiveInput; //!< range [0, 1]
};

class MainRotorComponent : public Component
{
public:
	MainRotorComponent(const MainRotorComponentConfig& config);

	void setNormalizedRpm(float rpm) { mDriverRpm = rpm; }

	float getPitchAngle() const {return mPitch;}

	void setRotationAngle(float angle) {mRotationAngle = angle;}
	float getRotationAngle() const {return mRotationAngle;}

	const Vector3& getPositionRelBody() const {return mPositionRelBody;}
	const Quaternion& getTppOrientationRelBody() const {return mTppOriRelBody;}

	float getTppPitchOffset() const { return mParams->tppPitchOffset; }

public: // SimUpdatable interface
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PreDynamicsSubStep, updatePreDynamicsSubstep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamicsSubstep();

private:
	float calculateInducedVelocity(float velSqLength) const;
	Vector3 calculateLift(float inducedVel) const;
	float calculateBladeElementLift(const Vector3& heliVelRelTpp, const Vector3& rotorVelRelTpp, int diskSector) const;

private:
	MainRotorParamsPtr mParams;
	Node* mNode;
	Motion* mMotion;
	DynamicBodyComponent* mBody;
	ControlInputVec2Ptr mCyclicInput; //!< range is [-1, 1]. Positive backward and right.
	ControlInputFloatPtr mCollectiveInput; //!< range [0, 1]

	const Vector3 mPositionRelBody;
	const Quaternion mOrientationRelBody;
	float mDriverRpm;

	float mPitch;
	float mDesiredPitch;
	float mRpm;
	float mRotationAngle;
	float mTppPitch; //!< Tip-plane-path pitch
	float mTppRoll; //!< Tip-plane-path roll
	Quaternion mTppOriRelBody;
	SecondsD mDt = 0;
};

} // namespace sim
} // namespace skybolt