/* Copyright Matthew Reid
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
	class View;
	class ViewerBase;
}

namespace skybolt {

class StatsDisplaySystem : public sim::System
{
public:
	//! Displays the viewer's stats on the given camera
	StatsDisplaySystem(osgViewer::ViewerBase* viewer, osgViewer::View* view, const osg::ref_ptr<osg::Camera>& camera);
	~StatsDisplaySystem();

	void setVisible(bool visible);

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::Attachments, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updateState();

private:
	osg::ref_ptr<osg::Camera> mCamera;
	osgViewer::View* mView;
	osg::Stats* mViewerStats;
	osg::Stats* mCameraStats;
	osg::ref_ptr<class VisHud> mStatsHud;
};

} // namespace skybolt