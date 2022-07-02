/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GpuTextureGenerator.h"

#include <osg/Camera>

namespace skybolt {
namespace vis {

class GpuTextureGeneratorDrawCallback : public osg::Camera::DrawCallback
{
public:
	GpuTextureGeneratorDrawCallback(GpuTextureGenerator* generator) : generator(generator) {}
	void operator() (const osg::Camera &) const
	{
		if (generator->mActive)
		{
			generator->mActive = false;
			generator->removeChild(generator->mCamera);
		}
	}

	GpuTextureGenerator* generator;
};

GpuTextureGenerator::GpuTextureGenerator(const osg::ref_ptr<osg::Texture2D>& texture, const osg::ref_ptr<osg::StateSet>& stateSet, bool generateMipMaps) :
	mTexture(texture),
	mActive(true)
{
	mCamera = new osg::Camera;
	mCamera->setClearMask(GL_COLOR_BUFFER_BIT); 
	mCamera->setViewport(0, 0, texture->getTextureWidth(), texture->getTextureHeight());
	mCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	mCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT); 
	mCamera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 0, generateMipMaps);
	mCamera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	mQuad.reset(new ScreenQuad(stateSet));
	mCamera->addChild(mQuad->_getNode());

	// TODO FIXME: There seems to be a bug in OSG where mipmaps are not used for generated textures unless the number of levels is explicitly specified
	if (generateMipMaps)
	{
		int mipMapCount = 1 + floor(log((float)osg::maximum(texture->getTextureWidth(), texture->getTextureHeight())));
		texture->setNumMipmapLevels(mipMapCount);
	}

	mCamera->setPostDrawCallback(new GpuTextureGeneratorDrawCallback(this)); // osg::Camera takes ownership of the callback by storing it as a ref_ptr

	addChild(mCamera);
}

GpuTextureGenerator::~GpuTextureGenerator()
{
	removeChild(mCamera);
}

void GpuTextureGenerator::requestRegenerate()
{
	if (!mActive)
	{
		mActive = true;
		addChild(mCamera);
	}
}

} // namespace vis
} // namespace skybolt
