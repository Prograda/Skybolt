/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "JulianDateSequenceController.h"
#include "SkyboltEngine/Sequence/Interpolator/LinearInterpolator.h"
#include <SkyboltEngine/Scenario.h>
#include <SkyboltSim/World.h>
#include <assert.h>

namespace skybolt {

JulianDateSequenceController::JulianDateSequenceController(const std::shared_ptr<DoubleStateSequence>& sequence, Scenario* scenario) :
	StateSequenceControllerT(sequence),
	mScenario(scenario)
{
	assert(mScenario);	mInterpolator = std::make_unique<LinearInterpolatorD>(
		[this] (int i) { return mSequence->values[i].value; });
}

SequenceStatePtr JulianDateSequenceController::getState() const
{
	return std::make_shared<DoubleSequenceState>(mScenario->startJulianDate);
}

void JulianDateSequenceController::setStateT(const DoubleSequenceState& state)
{
	mScenario->startJulianDate = state.value;
}

SequenceStatePtr JulianDateSequenceController::getStateAtInterpolationPoint(const math::InterpolationPoint& point) const
{
	double value = mInterpolator->interpolate(point.bounds.first, point.bounds.last, point.weight);
	return std::make_shared<DoubleSequenceState>(value);
}

} // namespace skybolt