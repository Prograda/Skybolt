/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

struct PropellerParams
{
	float rpmMultiplier;
	float pitchRange;
	float minPitch;
	float pitchResponseRate;
	float thrustPerRpmPerPitch; // Newtons per rpm per radian of pitch
};

struct PropellerComponentConfig
{
	PropellerParams params;
	Node* node;
	DynamicBodyComponent* body;
	Vector3 positionRelBody;
	Quaternion orientationRelBody;
	ControlInputFloatPtr input;
	float pitch;
};

class PropellerComponent : public Component
{
public:
	PropellerComponent(const PropellerComponentConfig& config);

	void updatePreDynamicsSubstep(TimeReal dt);

	void setDriverRpm(float rpm) {mDriverRpm = rpm;}

	float getPitchAngle() const {return mPitch;}

	void setRotationAngle(float angle) {mRotationAngle = angle;}
	float getRotationAngle() const {return mRotationAngle;}

	const Vector3& getPositionRelBody() const {return mPositionRelBody;}
	const Quaternion& getOrientationRelBody() const {return mOrientationRelBody;}

private:
	const PropellerParams mParams;
	Node* mNode;
	DynamicBodyComponent* mBody;
	Vector3 mPositionRelBody;
	Quaternion mOrientationRelBody;
	ControlInputFloatPtr mInput;
	float mDriverRpm;
	float mPitch;
	float mRotationAngle;
	float mRpm;
};

} // namespace sim
} // namespace skybolt