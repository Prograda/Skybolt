/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/WeightAveragedBuffer.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Chrono.h>

class SimUpdater
{
public:
	SimUpdater(const std::shared_ptr<skybolt::EngineRoot>& engineRoot);
	~SimUpdater();

	void update(skybolt::sim::SecondsD wallDt);

	double getRequestedTimeRate() const { return mRequestedTimeRate; }
	void setRequestedTimeRate(double rate) { mRequestedTimeRate = rate; }

	double getActualTimeRate() const { return mActualTimeRate; }

	void setMaxSimTimeStep(double dt) { mMaxSimDt = dt; }

protected:
	void simulate(skybolt::TimeSource& timeSource, float dt);

	std::shared_ptr<skybolt::EngineRoot> mEngineRoot;
	std::unique_ptr<skybolt::sim::SimStepper> mSimStepper;
	std::unique_ptr<skybolt::UniformAveragedBuffer> mAverageWallDt;
	double mMaxSimDt = 10;

	skybolt::sim::SecondsD mWallTime = 0;
	double mRequestedTimeRate = 1;
	double mActualTimeRate = 1;
};