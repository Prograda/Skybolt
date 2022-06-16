/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderTarget.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Scene.h"

namespace skybolt {
namespace vis {

RenderTarget::RenderTarget(const osg::ref_ptr<osg::Camera>& osgCamera) :
	mOsgCamera(osgCamera),
	mViewport(new osg::Viewport)
{
	assert(mOsgCamera);
	mOsgCamera->setViewport(mViewport);
	mRect = FixedRectIProvider(RectI(0, 0, mViewport->width(), mViewport->height()));

	mFarClipDistanceUniform = new osg::Uniform("farClipDistance", 1e5f);
	mOsgCamera->getOrCreateStateSet()->addUniform(mFarClipDistanceUniform);
}

void RenderTarget::setRect(const RectIProvider& rect)
{
	mRect = rect;
}

void RenderTarget::setCamera(const CameraPtr& camera)
{
	mCamera = camera;
}

void RenderTarget::setScene(const std::shared_ptr<Scene>& scene)
{
	if (mScene)
	{
		mOsgCamera->removeChild(mScene->getNode());
	}

	mScene = scene;

	if (scene)
	{
		mOsgCamera->addChild(scene->getNode());
	}
}

void RenderTarget::updatePreRender()
{
	RectI rect = mRect();

	osg::StateSet* ss = mOsgCamera->getOrCreateStateSet();
	mRcpWindowSizeInPixelsUniform = new osg::Uniform("rcpWindowSizeInPixels", osg::Vec2f(1.0f / rect.width, 1.0f / rect.height));
	ss->addUniform(mRcpWindowSizeInPixelsUniform);

	mViewport->setViewport(rect.x, rect.y, rect.width, rect.height);

	if (mCamera && mScene)
	{
		mCamera->setAspectRatio((float)mViewport->width() / (float)mViewport->height());
		mCamera->updateOsgCameraGeometry(*mOsgCamera);
		mScene->updatePreRender(*mCamera);

		mFarClipDistanceUniform->set(mCamera->getFarClipDistance());
	}
}

} // namespace vis
} // namespace skybolt
