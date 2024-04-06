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

SimStepper::~SimStepper() = default;

void SimStepper::step(SecondsD dt)
{
	auto systems = *mSystems; // Take copy in case a system adds/removes another system during step

	updateSystem(systems, UpdateStage::Input);
	updateSystem(systems, UpdateStage::BeginStateUpdate);

	if (mDynamicsEnabled && dt > 0)
	{
		updateDynamicsStep(systems, dt);
	}

	updateSystem(systems, UpdateStage::EndStateUpdate);
	updateSystem(systems, UpdateStage::Attachments);
	updateSystem(systems, UpdateStage::Output);
}

void SimStepper::updateDynamicsStep(const std::vector<SystemPtr>& systems, SecondsD dt)
{
	assert(mDynamicsEnabled);

	// Calculate required number of substeps
	double newStepTimer = mStepTimer + dt;

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
		updateSystem(systems, UpdateStage::PreDynamicsSubStep);

		for (const SystemPtr& system : *mSystems)
		{
			system->advanceSimTime(mCurrentTime, msDynamicsStepSize);
		}
		updateSystem(systems, UpdateStage::DynamicsSubStep);
		updateSystem(systems, UpdateStage::PostDynamicsSubStep);

		mCurrentTime += msDynamicsStepSize;
	}
}

void SimStepper::updateSystem(const std::vector<SystemPtr>& systems, UpdateStage stage)
{
	for (const SystemPtr& system : systems)
	{
		system->update(stage);
	}
}

} // namespace sim
} // namespace skybolt