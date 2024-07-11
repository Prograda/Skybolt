/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SequenceController.h"
#include "SkyboltEngine/Sequence/Interpolator/Interpolator.h"
#include <SkyboltSim/SimMath.h>

namespace skybolt {

struct DoubleSequenceState : public SequenceState
{
	DoubleSequenceState(double value) : value(value) {}
	std::string toString() const override { return std::to_string(value); }

	double value;
};

using DoubleStateSequence = StateSequenceT<DoubleSequenceState>;

class JulianDateSequenceController : public StateSequenceControllerT<DoubleSequenceState>
{
public:
	JulianDateSequenceController(const std::shared_ptr<DoubleStateSequence>& sequence, Scenario* scenario);
	SequenceStatePtr getState() const override;
	void setStateT(const DoubleSequenceState& state) override;
	SequenceStatePtr getStateAtInterpolationPoint(const math::InterpolationPoint& point) const override;

private:
	Scenario* mScenario;
	std::unique_ptr<InterpolatorD> mInterpolator;
};

} // namespace skybolt