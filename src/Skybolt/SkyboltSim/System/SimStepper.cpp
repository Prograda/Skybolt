/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimStepper.h"
#include "SkyboltSim/System/System.h"
#include <assert.h>

namespace skybolt {
namespace sim {

const double SimStepper::msDynamicsStepSize = 1.0 / 60.0;

SimStepper::SimStepper(const SystemRegistryPtr& systems) :
	mSystems(systems)
{
	assert(mSystems);
}

SimStepper::~SimStepper()
{
}

void SimStepper::step(const System::StepArgs& args)
{
	auto systems = *mSystems; // Take copy in case a system adds/removes another system during step

	for (const SystemPtr& system : systems)
	{
		system->updatePreDynamics(args);
	}

	updateDynamicsStep(args);

	for (const SystemPtr& system : systems)
	{
		system->updatePostDynamics(args);
	}
}

void SimStepper::updateDynamicsStep(const System::StepArgs& args)
{
	// Calculate required number of substeps
	double newStepTimer = mStepTimer + (double)args.dtSim;

	int requiredSteps = int(newStepTimer / msDynamicsStepSize);
	if (requiredSteps > msMaxDynamicsSubsteps)
	{
		requiredSteps = msMaxDynamicsSubsteps;
		double dt = (double)msMaxDynamicsSubsteps * msDynamicsStepSize;
		newStepTimer = mStepTimer + dt;
	}

	mStepTimer = newStepTimer - requiredSteps * msDynamicsStepSize;

	// Perform substeps
	for (int i = 0; i < requiredSteps; i++)
	{
		for (const SystemPtr& system : *mSystems)
		{
			system->updatePreDynamicsSubstep(msDynamicsStepSize);
		}

		for (const SystemPtr& system : *mSystems)
		{
			system->updateDynamicsSubstep(msDynamicsStepSize);
		}

		for (const SystemPtr& system : *mSystems)
		{
			system->updatePostDynamicsSubstep(msDynamicsStepSize);
		}
	}
}

} // namespace sim
} // namespace skybolt