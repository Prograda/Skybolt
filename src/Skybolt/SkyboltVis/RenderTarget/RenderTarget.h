/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderOperation.h"
#include "SkyboltVis/Rect.h"

#include <osg/Camera>
#include <osg/Group>

#include <functional>
#include <variant>

namespace skybolt {
namespace vis {

using RectIProvider = std::function<RectI()>;

inline RectIProvider FixedRectIProvider(const RectI& rect) { return [rect] { return rect; }; }

// Wraps an osg::Camera, and provides an interface for binding a vis::Camera and scene to the osg::Camera.
class RenderTarget : public RenderOperation
{
public:
	struct Scene
	{
		virtual ~Scene() {}
		virtual void updatePreRender(const Camera& camera) {}
		virtual osg::ref_ptr<osg::Node> getNode() = 0;
	};

	RenderTarget(const osg::ref_ptr<osg::Camera>& osgCamera);

	virtual void setRect(const RectIProvider& rect);

	virtual void setScene(const std::shared_ptr<Scene>& scene);

	virtual void setCamera(const CameraPtr& camera);
	CameraPtr getCamera() const { return mCamera; }

	osg::ref_ptr<osg::Camera> getOsgCamera() const { return mOsgCamera; }

protected:
	void updatePreRender() override;

protected:
	RectIProvider mRect;
	osg::ref_ptr<osg::Camera> mOsgCamera;
	osg::Viewport* mViewport;
	CameraPtr mCamera;
	std::shared_ptr<Scene> mScene;
	osg::Uniform* mRcpWindowSizeInPixelsUniform;
	osg::Uniform* mFarClipDistanceUniform;
};

} // namespace vis
} // namespace skybolt
