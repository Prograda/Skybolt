/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GpuTextureGenerator.h"
#include "TextureGeneratorCullCallback.h"

#include <osg/Camera>

namespace skybolt {
namespace vis {

GpuTextureGenerator::GpuTextureGenerator(const osg::ref_ptr<osg::Texture2D>& texture, const osg::ref_ptr<osg::StateSet>& stateSet, bool generateMipMaps) :
	mTextureGeneratorCullCallback(new TextureGeneratorCullCallback()),
	mTexture(texture)
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
	
	// Always cull the camera (don't render from the camera) after the first generation, unless the generator has been requested to regenerate.
	addCullCallback(mTextureGeneratorCullCallback);

	addChild(mCamera);
}

GpuTextureGenerator::~GpuTextureGenerator() = default;

void GpuTextureGenerator::requestRegenerate()
{
	mTextureGeneratorCullCallback->resetCulling();
}

} // namespace vis
} // namespace skybolt
