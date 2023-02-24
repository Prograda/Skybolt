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

namespace skybolt {
namespace vis {

RenderTexture::RenderTexture(const RenderTextureConfig& config) :
	RenderTarget(new osg::Camera),
	mColorTextureFactories(config.colorTextureFactories),
	mDepthTextureFactory(config.depthTextureFactory),
	mMultisampleSampleCount(config.multisampleSampleCount)
{
	mOsgCamera->setProjectionMatrix(osg::Matrix::ortho2D(-1, 1, -1, 1));
	mOsgCamera->setViewMatrix(osg::Matrix::identity());
	mOsgCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mOsgCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	mOsgCamera->setClearColor(osg::Vec4(0, 0, 0, 0));

	if (config.clear)
	{
		mOsgCamera->setClearMask(mDepthTextureFactory ? (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) : GL_COLOR_BUFFER_BIT);
	}
	else
	{
		mOsgCamera->setClearMask(0);
	}
	mOsgCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	mOsgCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	if (!mDepthTextureFactory)
	{
		// Ensure there is no depth buffer attached, otherwise we can't write to 3d texture layers.
		// OSG tries to automatically add a depth buffer for historical reasons, because
		// old drivers requred it. It's no longer needed. See https://groups.google.com/g/osg-users/c/_MksKA4Dmb4
		mOsgCamera->detach(osg::Camera::DEPTH_BUFFER);

		// Set implicit color buffer attachment, which prevents OSG from automatically adding a
		// depth buffer if one is not attached.
		mOsgCamera->setImplicitBufferAttachmentMask(osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT);
	}
}

RenderTexture::~RenderTexture()
{
}

void RenderTexture::updatePreRender(const RenderContext& renderContext)
{
	RenderTarget::updatePreRender(renderContext);

	int width = mOsgCamera->getViewport()->width();
	int height = mOsgCamera->getViewport()->height();

	if (mColorTextures.empty() || mColorTextures.front()->getTextureWidth() != width || mColorTextures.front()->getTextureHeight() != height)
	{
		mOsgCamera->setRenderingCache(nullptr); // Clear the cache so that attachments will update to use new textures

		int i = 0;
		mColorTextures.resize(mColorTextureFactories.size());
		for (const auto& factory : mColorTextureFactories)
		{
			mColorTextures[i] = factory(osg::Vec2i(width, height));
			mOsgCamera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0 + i), mColorTextures[i], 0, 0, false, mMultisampleSampleCount);
			++i;
		}

		if (mDepthTextureFactory)
		{
			mDepthTexture = (*mDepthTextureFactory)(osg::Vec2i(width, height));
			mOsgCamera->attach(osg::Camera::DEPTH_BUFFER, mDepthTexture);
		}
	}
}

TextureFactory createScreenTextureFactory(GLint internalFormat)
{
	return [internalFormat](const osg::Vec2i& size) {
		osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
		texture->setTextureSize(size.x(), size.y());
		texture->setResizeNonPowerOfTwoHint(false);
		texture->setInternalFormat(internalFormat);
		texture->setNumMipmapLevels(0);
		texture->setUseHardwareMipMapGeneration(false);
		texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR);
		texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

		return texture;
	};
}

} // namespace vis
} // namespace skybolt
