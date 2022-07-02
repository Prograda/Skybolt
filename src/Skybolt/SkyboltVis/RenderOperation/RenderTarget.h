/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/RenderOperation/RenderOperation.h"
#include "SkyboltVis/Rect.h"

#include <osg/Camera>
#include <osg/Group>

#include <functional>
#include <variant>

namespace skybolt {
namespace vis {

// A RenderOperation that renders a scene using a vis::Camera
class RenderTarget : public RenderOperation
{
public:
	RenderTarget(const osg::ref_ptr<osg::Camera>& osgCamera);

	virtual void setRelativeRect(const RectF& rect); //!< Sets rect size relative to parent render target (usually the window). Default is full rect.

	virtual void setScene(const osg::ref_ptr<osg::Node>& scene);

	virtual void setCamera(const CameraPtr& camera);
	CameraPtr getCamera() const { return mCamera; }

	osg::ref_ptr<osg::Camera> getOsgCamera() const { return mOsgCamera; }

protected:
	void updatePreRender(const RenderContext& renderContext) override;

protected:
	RectF mRect;
	osg::ref_ptr<osg::Camera> mOsgCamera;
	osg::Viewport* mViewport;
	CameraPtr mCamera;
	osg::ref_ptr<osg::Node> mScene;
	osg::Uniform* mRcpWindowSizeInPixelsUniform;
};

//! Render target that renders into the currently attached frame buffer,
//!  which is usually the backbuffer unless the target exists under an osg::camera rendering into a different frame buffer.
osg::ref_ptr<RenderTarget> createDefaultRenderTarget();

} // namespace vis
} // namespace skybolt
