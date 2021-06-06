/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "RenderTexture.h"
#include "Camera.h"
#include "SkyboltVis/Window/Window.h"

#include <osg/Camera>
#include <osg/Texture2D>

using namespace skybolt::vis;

RenderTexture::RenderTexture(const RenderTextureConfig& config) :
	RenderTarget(new osg::Camera),
	mColorTextureFactory(config.colorTextureFactory),
	mDepthTextureFactory(config.depthTextureFactory),
	mMultisampleSampleCount(config.multisampleSampleCount)
{
	mOsgCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mOsgCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	mOsgCamera->setClearColor(osg::Vec4(0, 0, 0, 0));

	mOsgCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mOsgCamera->setViewport(0, 0, 400, 300);
	mOsgCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	mOsgCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	addChild(mOsgCamera);
}

RenderTexture::~RenderTexture()
{
}

void RenderTexture::updatePreRender()
{
	int width = mOsgCamera->getViewport()->width();
	int height = mOsgCamera->getViewport()->height();

	if (!mTexture || mTexture->getTextureWidth() != width || mTexture->getTextureHeight() != height)
	{
		mOsgCamera->setRenderingCache(nullptr); // Clear the cache so that attachments will update to use new textures

		mTexture = mColorTextureFactory(osg::Vec2i(width, height));
		mOsgCamera->attach(osg::Camera::COLOR_BUFFER, mTexture, 0, 0, false, mMultisampleSampleCount);
		colorTextureCreated(mTexture);

		if (mDepthTextureFactory)
		{
			auto texture = (*mDepthTextureFactory)(osg::Vec2i(width, height));
			mOsgCamera->attach(osg::Camera::DEPTH_BUFFER, texture);
			depthTextureCreated(texture);
		}
		mOsgCamera->setViewport(0, 0, width, height);
	}

	RenderTarget::updatePreRender();
}