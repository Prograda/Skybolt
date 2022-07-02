/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "StatsDisplaySystem.h"
#include "SkyboltEngine/VisHud.h"
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Window/Window.h>
#include <osgViewer/Viewer>

namespace skybolt {

StatsDisplaySystem::StatsDisplaySystem(osgViewer::Viewer* viewer, const osg::ref_ptr<osg::Camera>& camera) :
	mCamera(camera)
{
	mViewerStats = viewer->getStats();
	mCameraStats = viewer->getCamera()->getStats();

	mViewerStats->collectStats("frame_rate", true);
	mCameraStats->collectStats("gpu", true);

	if (false)
	{
		mViewerStats->collectStats("event", true);
		mViewerStats->collectStats("update", true);
		mCameraStats->collectStats("rendering", true);
		mCameraStats->collectStats("scene", true);
	}

	mStatsHud = osg::ref_ptr<VisHud>(new VisHud());
	mCamera->addChild(mStatsHud);
}

StatsDisplaySystem::~StatsDisplaySystem()
{
	setVisible(false);
}

void StatsDisplaySystem::setVisible(bool visible)
{
	bool currentlyVisible = (mStatsHud->getNumParents() > 0);
	if (visible && !currentlyVisible)
	{
		mCamera->addChild(mStatsHud);
	}
	else if (!visible && currentlyVisible)
	{
		mCamera->removeChild(mStatsHud);
	}
}

void StatsDisplaySystem::updatePostDynamics(const System::StepArgs& args)
{
	osg::Viewport* viewport = mCamera->getViewport();
	mStatsHud->setAspectRatio(viewport->width() / viewport->height());

	mStatsHud->clear();

	int line = 0;
	int frameNumber = mViewerStats->getLatestFrameNumber() - 1;

	auto attributes = mViewerStats->getAttributeMap(frameNumber);
	auto attributes2 = mCameraStats->getAttributeMap(frameNumber);
	attributes.insert(attributes2.begin(), attributes2.end());

	const float lineHeight = 0.05f;
	const float textSize = lineHeight * 0.8f;

	for (const auto& value : attributes)
	{
		mStatsHud->drawText(glm::vec2(-0.9f, 0.9f - line * lineHeight), value.first + ": " + std::to_string(value.second), 0.0f, textSize);
		++line;
	}
}

} // namespace skybolt