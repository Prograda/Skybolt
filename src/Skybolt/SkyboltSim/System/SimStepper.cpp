/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimStepper.h"
#include "SkyboltSim/System/System.h"
#include <assert.h>

namespace skybolt {
namespace sim {

SimStepper::SimStepper(const SystemRegistryPtr& systems, const TimeRange& range) :
	TimeSource(range),
	mSystems(systems)
{
	assert(mSystems);
}

SimStepper::~SimStepper() = default;

void SimStepper::setTime(SecondsD t)
{
	SecondsD prevTime = mTime;
	TimeSource::setTime(t);
	if (mTime != prevTime)
	{
		for (const SystemPtr& system : *mSystems)
		{
			system->setSimTime(mTime);
		}
	}
}

void SimStepper::advanceTime(SecondsD dt)
{
	auto systems = *mSystems; // Take copy in case a system adds/removes another system during step

	updateSystem(systems, UpdateStage::Input);
	updateSystem(systems, UpdateStage::BeginStateUpdate);

	if (dt > 0)
	{
		if (mDynamicsEnabled)
		{
			advanceTimeByDynamicsSubSteps(systems, dt);
		}
		else
		{
			advanceTimeByNonDynamicsStep(systems, dt);
		}
	}

	updateSystem(systems, UpdateStage::EndStateUpdate);
	updateSystem(systems, UpdateStage::Attachments);
	updateSystem(systems, UpdateStage::Output);
}

void SimStepper::advanceToTime(sim::SecondsD time)
{
	SecondsD dt = time - mTime;
	if (dt > 0)
	{
		// Advance time without limiting the number of dynamics substeps, since this is a "jump" in time and we want to ensure we reach the target time.
		std::optional<int> oldMaxDynamicsSubsteps = mMaxDynamicsSubsteps;
		mMaxDynamicsSubsteps = std::nullopt;
		advanceTime(dt);
		mMaxDynamicsSubsteps = oldMaxDynamicsSubsteps;
	}
}

void SimStepper::advanceTimeByDynamicsSubSteps(const std::vector<SystemPtr>& systems, SecondsD dt)
{
	assert(mDynamicsEnabled);

	// Calculate required number of substeps
	double newStepTimer = mStepTimer + dt;

	int requiredSteps = int(newStepTimer / mDynamicsStepSize);
	if (mMaxDynamicsSubsteps && requiredSteps > *mMaxDynamicsSubsteps)
	{
		requiredSteps = *mMaxDynamicsSubsteps;
		double limitedDt = (double)*mMaxDynamicsSubsteps * mDynamicsStepSize;
		newStepTimer = mStepTimer + limitedDt;
	}

	mStepTimer = newStepTimer - requiredSteps * mDynamicsStepSize;

	// Perform substeps
	for (int i = 0; i < requiredSteps; i++)
	{
		SecondsD prevTime = mTime;
		TimeSource::advanceTime(mDynamicsStepSize);

		if (mTime == prevTime)
		{
			// Time did not advance, so we are at the end of the time range. Stop stepping.
			break;
		}

		updateSystem(systems, UpdateStage::PreDynamicsSubStep);

		for (const SystemPtr& system : *mSystems)
		{
			system->advanceSimTime(mTime, mDynamicsStepSize);
		}
		updateSystem(systems, UpdateStage::DynamicsSubStep);
		updateSystem(systems, UpdateStage::PostDynamicsSubStep);
	}
}

void SimStepper::advanceTimeByNonDynamicsStep(const std::vector<SystemPtr>& systems, SecondsD dt)
{
	assert(!mDynamicsEnabled);

	// Dynamics is disabled, so just advance time by dt without taking substeps
	TimeSource::advanceTime(dt);

	for (const SystemPtr& system : *mSystems)
	{
		system->advanceSimTime(mTime, dt);
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