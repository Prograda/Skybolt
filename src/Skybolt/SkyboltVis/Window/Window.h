/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderOperationSequence.h"

#include <osgViewer/View>
#include <set>

namespace skybolt {
namespace vis {

class Window
{
	friend class VisRoot;
public:
	Window(const osg::ref_ptr<osgViewer::View>& view);
	virtual ~Window();

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	osg::ref_ptr<osgViewer::View> getView() const { return mView; }

	RenderOperationSequence& getRenderOperationSequence() { return *mRenderOperationSequence; }
	const RenderOperationSequence& getRenderOperationSequence() const { return *mRenderOperationSequence; }

protected:
	void setLoadTimingPolicy(LoadTimingPolicy loadTimingPolicy) { mLoadTimingPolicy = loadTimingPolicy; }
	void preRender(LoadTimingPolicy policy);

protected:
	osg::ref_ptr<osgViewer::View> mView;

private:
	std::unique_ptr<RenderOperationSequence> mRenderOperationSequence;
	osg::Uniform* mScreenSizePixelsUniform;
	int mFrameNumber = 0;
	LoadTimingPolicy mLoadTimingPolicy = LoadTimingPolicy::LoadAcrossMultipleFrames;
};

void configureGraphicsState(osg::GraphicsContext& context);

} // namespace vis
} // namespace skybolt
