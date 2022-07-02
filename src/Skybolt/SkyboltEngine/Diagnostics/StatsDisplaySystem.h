/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltSim/System/System.h"
#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Camera>
#include <osg/ref_ptr>
#include <osg/Stats>

namespace osgViewer {
	class Viewer;
}

namespace skybolt {

class StatsDisplaySystem : public sim::System
{
public:
	//! Displays the viewer's stats on the given camera
	StatsDisplaySystem(osgViewer::Viewer* viewer, const osg::ref_ptr<osg::Camera>& camera);
	~StatsDisplaySystem();

	void setVisible(bool visible);

	void updatePostDynamics(const System::StepArgs& args) override;

private:
	osg::ref_ptr<osg::Camera> mCamera;
	osg::Stats* mViewerStats;
	osg::Stats* mCameraStats;
	osg::ref_ptr<class VisHud> mStatsHud;
};

} // namespace skybolt