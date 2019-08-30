/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/System/SystemRegistry.h"
#include "System.h"
#include <vector>

namespace skybolt {
namespace sim {

class SimStepper
{
public:
	SimStepper(const SystemRegistryPtr& systems);
	~SimStepper();

	void step(const System::StepArgs& args);

private:
	void updateDynamicsStep(const System::StepArgs& args);

private:
	SystemRegistryPtr mSystems;
	double mStepTimer = 0;
	static const double msDynamicsStepSize;
	static const int msMaxDynamicsSubsteps = 10;
};

} // namespace sim
} // namespace skybolt