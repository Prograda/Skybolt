/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CloudsTemporalUpscaling.h"
#include "VolumeClouds.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Renderable/Clouds/VolumeClouds.h"
#include "SkyboltVis/Renderable/Planet/Planet.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"

namespace skybolt {
namespace vis {

CloudsTemporalUpscaling::CloudsTemporalUpscaling(const CloudsTemporalUpscalingConfig& config) :
	mScene(config.scene),
	mColorTextureProvider(config.colorTextureProvider),
	mDepthTextureProvider(config.depthTextureProvider)
{
	mReprojectionMatrixUniform = new osg::Uniform("reprojectionMatrix", osg::Matrixf());
	mInvReprojectionMatrixUniform = new osg::Uniform("invReprojectionMatrix", osg::Matrixf());
	mFrameNumberUniform = new osg::Uniform("frameNumber", 0);
	mJitterOffsetUniform = new osg::Uniform("jitterOffset", osg::Vec2f(0.f, 0.f));

	{
		mStateSet = new osg::StateSet();
		mStateSet->setAttributeAndModes(config.upscalingProgram, osg::StateAttribute::ON);

		mStateSet->addUniform(createUniformSampler2d("colorTextureSrc", 0));
		mStateSet->addUniform(createUniformSampler2d("depthTextureSrc", 1));
		mStateSet->addUniform(createUniformSampler2d("colorTexturePrev", 2));
		mStateSet->addUniform(createUniformSampler2d("depthTexturePrev", 3));

		mStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		mStateSet->addUniform(mReprojectionMatrixUniform);
		mStateSet->addUniform(mInvReprojectionMatrixUniform);
		mStateSet->addUniform(mFrameNumberUniform);
		mStateSet->addUniform(mJitterOffsetUniform);
	}

	osg::ref_ptr<osg::Node> quad = ScreenQuad::createNode(mStateSet);

	for (int i = 0; i < 2; ++i)
	{
		RenderTextureConfig textureConfig;
		textureConfig.clear = false;
		textureConfig.colorTextureFactories = { createScreenTextureFactory(GL_RGBA16), createScreenTextureFactory(GL_R32F) };
		mUpscaledColorTexture[i] = new RenderTexture(textureConfig);
		mUpscaledColorTexture[i]->setScene(quad);
	}
}

// https://en.wikipedia.org/wiki/Halton_sequence#Implementation_in_pseudocode
float halton(int base, int index)
{
	float result = 0;
	float f = 1;
	while (index > 0)
	{
		f = f / float(base);
		result += f * float(index % base);
		index = index / base; 
	}
	return result;
}

const int ditherMatrix4x4[] = {
	0, 8, 2, 10, 
	12, 4, 14, 6,
	3, 11, 1, 9,
	15, 7, 13, 5
};

void CloudsTemporalUpscaling::updatePreRender(const RenderContext& context)
{
	VolumeCloudsPtr clouds;
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		clouds = planet->getCloudsVisible() ? planet->getClouds() : nullptr;
	}

	if (!clouds || !mCamera)
	{
		removeChildren(0, getNumChildren());
		return;
	}

	for (int i = 0; i < 2; ++i)
	{
		mUpscaledColorTexture[i]->updatePreRender(context);
	}

	mUpscaledOutputIndex = 1 - mUpscaledOutputIndex;

	osg::ref_ptr<RenderTexture> input = mUpscaledColorTexture[1 - mUpscaledOutputIndex];
	osg::ref_ptr<RenderTexture> output = mUpscaledColorTexture[mUpscaledOutputIndex];

	mStateSet->setTextureAttribute(0, mColorTextureProvider(), osg::StateAttribute::ON);
	mStateSet->setTextureAttribute(1, mDepthTextureProvider(), osg::StateAttribute::ON);
	mStateSet->setTextureAttribute(2, input->getOutputTextures().front(), osg::StateAttribute::ON);
	mStateSet->setTextureAttribute(3, input->getOutputTextures().back(), osg::StateAttribute::ON);
	removeChild(input);
	addChild(output);

	osg::Matrix modelMatrix = clouds->getModelMatrix();

	const Camera& camera = *mCamera;
	// Update uniforms
	{
		osg::Vec3f cameraPosition = camera.getPosition();

		osg::Matrix proj = camera.getProjectionMatrix();

		osg::Matrix projXy = osg::Matrix::identity();
		projXy(0, 0) = proj(0, 0);
		projXy(1, 1) = proj(1, 1);
		osg::Matrix modelViewProjXy = modelMatrix * camera.getViewMatrix() * projXy;

		mReprojectionMatrixUniform->set(osg::Matrix::inverse(modelViewProjXy) * mPrevFrameModelViewProjXyMatrix);
		mInvReprojectionMatrixUniform->set(osg::Matrix::inverse(mPrevFrameModelViewProjXyMatrix) * modelViewProjXy);
		mPrevFrameModelViewProjXyMatrix = modelViewProjXy;
			
		mFrameNumberUniform->set(context.frameNumber);
		mJitterOffsetUniform->set(clouds->getCurrentFrameJitterNdcOffset());
	}
}

std::vector<osg::ref_ptr<osg::Texture>> CloudsTemporalUpscaling::getOutputTextures() const
{
	return mUpscaledColorTexture[mUpscaledOutputIndex]->getOutputTextures();
}

} // namespace vis
} // namespace skybolt
