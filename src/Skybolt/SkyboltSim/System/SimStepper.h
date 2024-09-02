/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/System/SystemRegistry.h"
#include "System.h"

#include <optional>
#include <vector>

namespace skybolt {
namespace sim {

class SimStepper
{
public:
	SimStepper(const SystemRegistryPtr& systems);
	~SimStepper();

	void setTime(SecondsD t);
	SecondsD getTime() const { return mCurrentTime; }

	void update(SecondsD dt);

	void setDynamicsEnabled(bool enabled) { mDynamicsEnabled = enabled; }

	void setDynamicsStepSize(double stepSize) { mDynamicsStepSize = stepSize; }
	void setMaxDynamicsSubsteps(const std::optional<int>& substeps) { mMaxDynamicsSubsteps = substeps; }

private:
	void updateDynamicsStep(const std::vector<SystemPtr>& systems, SecondsD dt);

	void updateSystem(const std::vector<SystemPtr>& systems, UpdateStage stage);

private:
	SystemRegistryPtr mSystems;
	SecondsD mCurrentTime = 0;
	SecondsD mStepTimer = 0;
	bool mDynamicsEnabled = true;

	double mDynamicsStepSize = 1.0 / 60.0;
	std::optional<int> mMaxDynamicsSubsteps = 10;
};

} // namespace sim
} // namespace skybolt