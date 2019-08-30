/* Copyright 2012-2020 Matthew Reid
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

	void updatePreDynamicsSubstep(TimeReal dt);
	float getRpm() const {return mEngineRpm;}

private:
	const JetTurbineParams mParams;
	ControlInputFloatPtr mInput;
	float mEngineRpm;
};

} // namespace sim
} // namespace skybolt