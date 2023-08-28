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

	osg::StateSet* ss = mOsgCamera->getOrCreateStateSet();

	mRcpWindowSizeInPixelsUniform = new osg::Uniform("rcpWindowSizeInPixels", osg::Vec2f(1.0f, 1.0f));
	ss->addUniform(mRcpWindowSizeInPixelsUniform);

	mRcpViewAspectRatio = new osg::Uniform("rcpViewAspectRatio", 1.0f);
	ss->addUniform(mRcpViewAspectRatio);
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
		std::ceil(mRect.width * context.targetDimensions.x()), // ceil to ensure required width is covered
		std::ceil(mRect.height * context.targetDimensions.y()));

	osg::StateSet* ss = mOsgCamera->getOrCreateStateSet();
	mRcpWindowSizeInPixelsUniform->set(osg::Vec2f(1.0f / rect.width, 1.0f / rect.height));
	mRcpViewAspectRatio->set(float(rect.height) / float(rect.width));

	mViewport->setViewport(rect.x, rect.y, rect.width, rect.height);

	if (mCamera)
	{
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
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return new RenderTarget(camera);
}

} // namespace vis
} // namespace skybolt
