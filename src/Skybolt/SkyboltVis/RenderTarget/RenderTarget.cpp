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

class RenderTargetNodeCallback : public osg::NodeCallback
{
public:
	RenderTargetNodeCallback(RenderTarget* target) : target(target) {}
	void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		target->updatePreRender();
	}

private:
	RenderTarget* target;
};

RenderTarget::RenderTarget(const osg::ref_ptr<osg::Camera>& osgCamera) :
	mOsgCamera(osgCamera),
	mViewport(new osg::Viewport)
{
	assert(mOsgCamera);
	setUpdateCallback(new RenderTargetNodeCallback(this));

	mFarClipDistanceUniform = new osg::Uniform("farClipDistance", 1e5f);
	mOsgCamera->getOrCreateStateSet()->addUniform(mFarClipDistanceUniform);
}

void RenderTarget::setRect(const RectI& rect)
{
	osg::StateSet* ss = mOsgCamera->getOrCreateStateSet();
	mRcpWindowSizeInPixelsUniform = new osg::Uniform("rcpWindowSizeInPixels", osg::Vec2f(0, 0));
	ss->addUniform(mRcpWindowSizeInPixelsUniform);

	mRcpWindowSizeInPixelsUniform->set(osg::Vec2f(1.0f / rect.width, 1.0f / rect.height));

	mViewport->setViewport(rect.x, rect.y, rect.width, rect.height);
	mOsgCamera->setViewport(mViewport);
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
