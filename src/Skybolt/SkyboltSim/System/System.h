/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace sim {

class System
{
public:
	virtual ~System() {}

	struct StepArgs
	{
		double dtSim;
		double dtWallClock;
	};

	virtual void updatePreDynamics(const StepArgs& args) {};
	virtual void updatePreDynamicsSubstep(double dtSubstep) {};
	virtual void updateDynamicsSubstep(double dtSubstep) {};
	virtual void updatePostDynamicsSubstep(double dtSubstep) {};
	virtual void updatePostDynamics(const StepArgs& args) {};
};

} // namespace sim
} // namespace skybolt