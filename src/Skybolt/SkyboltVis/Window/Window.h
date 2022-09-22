/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderOperationSequence.h"

#include <osgViewer/Viewer>
#include <set>

namespace skybolt {
namespace vis {

class Window
{
public:
	Window(std::unique_ptr<osgViewer::Viewer> viewer, const DisplaySettings& config);
	~Window();

	//! @returns false if window has been closed
	virtual bool render(LoadTimingPolicy loadTimingPolicy = LoadTimingPolicy::LoadAcrossMultipleFrames);

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	osgViewer::Viewer& getViewer() const { return *mViewer; }
	std::weak_ptr<osgViewer::Viewer> getViewerPtr() const {return mViewer;}

	RenderOperationSequence& getRenderOperationSequence() { return *mRenderOperationSequence; }
	const RenderOperationSequence& getRenderOperationSequence() const { return *mRenderOperationSequence; }

protected:
	void configureGraphicsState();

protected:
	std::shared_ptr<osgViewer::Viewer> mViewer;

private:
	osg::ref_ptr<osg::Group> mRootGroup;
	std::unique_ptr<RenderOperationSequence> mRenderOperationSequence;
	osg::Uniform* mScreenSizePixelsUniform;
};

} // namespace vis
} // namespace skybolt
