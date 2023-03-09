/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "JetTurbineComponent.h"
#include <algorithm>

namespace skybolt {
namespace sim {

JetTurbineComponent::JetTurbineComponent(const JetTurbineParams& params, const ControlInputFloatPtr& input) :
	mParams(params),
	mInput(input),
	mEngineRpm(0.0f)
{
	assert(mInput);
}

void JetTurbineComponent::updatePreDynamicsSubstep(TimeReal dt)
{
	// First order lag function
	float delta = (mInput->value - mEngineRpm) * mParams.rpmResponseRate;
	mEngineRpm += std::min(mParams.maxRpmDelta, std::max(mParams.minRpmDelta, delta)) * dt;
}

} // namespace sim
} // namespace skybolt