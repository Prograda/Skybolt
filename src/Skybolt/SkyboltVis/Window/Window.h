/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/RenderTarget/RenderOperationPipeline.h"

#include <osgViewer/Viewer>
#include <set>

namespace skybolt {
namespace vis {

class Window
{
public:
	Window(const DisplaySettings& config);
	~Window();

	//! @returns false if window has been closed
	virtual bool render();

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	osgViewer::Viewer& getViewer() const { return *mViewer; }
	std::weak_ptr<osgViewer::Viewer> getViewerPtr() const {return mViewer;}

	void setRenderOperationPipeline(const RenderOperationPipelinePtr& renderOperationPipeline);
	RenderOperationPipelinePtr& getRenderOperationPipeline() { return mRenderOperationPipeline; }
	const RenderOperationPipelinePtr& getRenderOperationPipeline() const { return mRenderOperationPipeline; }

protected:
	void configureGraphicsState();
	osg::Group* getSceneGraphRoot() const;

protected:
	std::shared_ptr<osgViewer::Viewer> mViewer;

private:
	osg::ref_ptr<osg::Group> mRootGroup;
	RenderOperationPipelinePtr mRenderOperationPipeline;
	CameraPtr mCamera;
	osg::Uniform* mScreenSizePixelsUniform;

	std::vector<osg::ref_ptr<RenderOperation>> mOperations;
};

//! @return the RenderTarget representing the final frame output 
osg::ref_ptr<RenderTarget> getFinalRenderTarget(const Window& window);

} // namespace vis
} // namespace skybolt
