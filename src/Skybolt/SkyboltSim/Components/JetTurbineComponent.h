/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"

namespace skybolt {
namespace sim {

struct JetTurbineParams
{
	float rpmResponseRate;
	float maxRpmDelta;
	float minRpmDelta;
};

class JetTurbineComponent : public Component
{
public:
	JetTurbineComponent(const JetTurbineParams& params, const ControlInputFloatPtr &input);

	float getRpm() const {return mEngineRpm;}

public: // SimUpdatable interface
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PreDynamicsSubStep, updatePreDynamicsSubstep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamicsSubstep();

private:
	const JetTurbineParams mParams;
	ControlInputFloatPtr mInput;
	float mEngineRpm;
	SecondsD mDt = 0;
};

} // namespace sim
} // namespace skybolt