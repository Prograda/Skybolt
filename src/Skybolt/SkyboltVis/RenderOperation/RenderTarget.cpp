/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderTarget.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Scene.h"

#include <assert.h>

namespace skybolt {
namespace vis {

RenderTarget::RenderTarget(const osg::ref_ptr<osg::Camera>& osgCamera) :
	mOsgCamera(osgCamera),
	mViewport(new osg::Viewport),
	mRect(0, 0, 1, 1)
{
	assert(mOsgCamera);
	mOsgCamera->setViewport(mViewport);
	addChild(mOsgCamera);
}

void RenderTarget::setRelativeRect(const RectF& rect)
{
	mRect = rect;
}

void RenderTarget::setCamera(const CameraPtr& camera)
{
	mCamera = camera;
}

void RenderTarget::setScene(const osg::ref_ptr<osg::Node>& scene)
{
	if (mScene)
	{
		mOsgCamera->removeChild(mScene);
	}

	mScene = scene;

	if (scene)
	{
		mOsgCamera->addChild(scene);
	}
}

void RenderTarget::updatePreRender(const RenderContext& context)
{
	RectF rect(
		mRect.x * context.targetDimensions.x(),
		mRect.y * context.targetDimensions.y(),
		mRect.width * context.targetDimensions.x(),
		mRect.height * context.targetDimensions.y());

	osg::StateSet* ss = mOsgCamera->getOrCreateStateSet();
	mRcpWindowSizeInPixelsUniform = new osg::Uniform("rcpWindowSizeInPixels", osg::Vec2f(1.0f / rect.width, 1.0f / rect.height));
	ss->addUniform(mRcpWindowSizeInPixelsUniform);

	mViewport->setViewport(rect.x, rect.y, rect.width, rect.height);

	if (mCamera)
	{
		mCamera->setAspectRatio((float)context.targetDimensions.x() / (float)context.targetDimensions.y());

		mCamera->updateOsgCameraGeometry(*mOsgCamera);
	}
}

osg::ref_ptr<RenderTarget> createDefaultRenderTarget()
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	camera->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_SIDES_CULLING);
	camera->setClearColor(osg::Vec4(0, 0, 0, 0));

	return new RenderTarget(camera);
}

} // namespace vis
} // namespace skybolt
