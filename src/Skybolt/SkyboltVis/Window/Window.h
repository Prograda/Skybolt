/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/RenderTarget/Viewport.h"

#include <osgViewer/Viewer>
#include <set>

namespace skybolt {
namespace vis {

class Window
{
public:
	Window();
	~Window();

	//! @returns false if window has been closed
	virtual bool render();

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	osgViewer::Viewer& getViewer() const { return *mViewer; }
	std::weak_ptr<osgViewer::Viewer> getViewerPtr() const {return mViewer;}

	//! @param rect is dimensions normalized by parent window dimensions. Range is [0, 1]
	void addRenderTarget(const osg::ref_ptr<RenderTarget>& target, const RectF& rect);
	void removeRenderTarget(const osg::ref_ptr<RenderTarget>& target);

	struct Target
	{
		Target(const osg::ref_ptr<RenderTarget>& target, const RectF& rect) : target(target), rect(rect) {}
		osg::ref_ptr<RenderTarget> target;
		RectF rect;
	};

	const std::vector<Target>& getRenderTargets() const { return mTargets; }

protected:
	void configureGraphicsState();
	osg::Group* getSceneGraphRoot() const;

protected:
	std::shared_ptr<osgViewer::Viewer> mViewer;

private:
	CameraPtr mCamera;
	osg::Uniform* mScreenSizePixelsUniform;

	std::vector<Target> mTargets;
};

} // namespace vis
} // namespace skybolt
