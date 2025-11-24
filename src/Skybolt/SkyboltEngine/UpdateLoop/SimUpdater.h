#pragma once

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltCommon/WeightAveragedBuffer.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Chrono.h>

class SimUpdater
{
public:
	SimUpdater(skybolt::NonNullPtr<skybolt::EngineRoot> engineRoot);
	~SimUpdater();

	void update(skybolt::sim::SecondsD wallDt);

	double getRequestedTimeRate() const { return mRequestedTimeRate; }
	void setRequestedTimeRate(double rate) { mRequestedTimeRate = rate; }

	double getActualTimeRate() const { return mActualTimeRate; }

	void setMaxSimTimeStep(double dt) { mMaxSimDt = dt; }

protected:
	void advanceWallTime(skybolt::sim::SecondsD wallDt);
	void simulate(skybolt::TimeSource& timeSource, skybolt::sim::SecondsD dt);

	const skybolt::NonNullPtr<skybolt::EngineRoot> mEngineRoot;
	const std::unique_ptr<skybolt::sim::SimStepper> mSimStepper;

	std::unique_ptr<skybolt::UniformAveragedBuffer> mAverageWallDt;
	double mMaxSimDt = 10;

	skybolt::sim::SecondsD mWallTime = 0;
	double mRequestedTimeRate = 1;
	double mActualTimeRate = 1;
};