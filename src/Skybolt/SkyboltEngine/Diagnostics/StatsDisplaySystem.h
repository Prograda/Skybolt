/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltSim/System/System.h"
#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Stats>

namespace skybolt {

class StatsDisplaySystem : public sim::System
{
public:
	StatsDisplaySystem(const vis::Window& window, vis::Scene& Scene);

	void updatePostDynamics(const System::StepArgs& args) override;

private:
	osg::Stats* mViewerStats;
	osg::Stats* mCameraStats;
	std::unique_ptr<VisHud> mStatsHud;
};

} // namespace skybolt