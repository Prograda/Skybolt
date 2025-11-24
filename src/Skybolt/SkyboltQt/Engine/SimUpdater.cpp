/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimUpdater.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltSim/System/SimStepper.h>

using namespace skybolt;
using namespace skybolt::sim;

const SecondsD maxWallDt = 0.2;

SimUpdater::SimUpdater(const std::shared_ptr<skybolt::EngineRoot>& engineRoot) :
	mEngineRoot(engineRoot),
	mSimStepper(std::make_unique<SimStepper>(engineRoot->systemRegistry)),
	mAverageWallDt(std::make_unique<UniformAveragedBuffer>(16))
{
	mSimStepper->setMaxDynamicsSubsteps(std::nullopt);
}

SimUpdater::~SimUpdater() = default;

void SimUpdater::update(SecondsD wallDt)
{
	bool isLive = mEngineRoot->scenario->timelineMode.get() == TimelineMode::Live;
	mSimStepper->setDynamicsEnabled(isLive);

	const auto& timeSource = mEngineRoot->scenario->timeSource;

	// Update simulation to sim time from time source.
	// This is necessary because the current time may have changed since the last update,
	// for example if we jumped to a different point on the timeline.
	SecondsD simTime = timeSource.getTime();
	if (mSimStepper->getTime() != simTime)
	{
		// Update SimStepper to the current time.
		mSimStepper->setTime(simTime);
	}

	// Advance forward time
	if (wallDt > 0)
	{
		advanceWallTime(wallDt);
	}

	// TimeSource time should equal SimStepper time after update
	assert (timeSource.getTime() == mSimStepper->getTime());
}

void SimUpdater::advanceWallTime(SecondsD wallDt)
{
	// Calculate simulation delta time
	double simDt;
	TimeSource& timeSource = mEngineRoot->scenario->timeSource;
	if (timeSource.getState() == TimeSource::StatePlaying)
	{
		mAverageWallDt->addValue(wallDt);
		double averageWallDt = mAverageWallDt->getResult();

		double maxTimeRate = mActualTimeRate * maxWallDt / averageWallDt;
		mActualTimeRate = std::min(mRequestedTimeRate, maxTimeRate);
		simDt = std::min(averageWallDt * mActualTimeRate, mMaxSimDt);
		mActualTimeRate = simDt / averageWallDt;
	}
	else
	{
		simDt = 0;
	}

	// Simulate by dt.
	// Note: we still need to simulate even if dt is 0, because some systems/components
	// need to still be updated even when the simulation is paused, e.g. in the editor.
	simulate(timeSource, simDt);

	// Advance wallclock time
	for (const SystemPtr& system : *mEngineRoot->systemRegistry)
	{
		system->advanceWallTime(mWallTime, wallDt);
	}

	mWallTime += wallDt;
}

void SimUpdater::simulate(TimeSource& timeSource, SecondsD dt)
{
	assert(dt >= 0);
	mSimStepper->update(dt);
	timeSource.setTime(mSimStepper->getTime());
}
