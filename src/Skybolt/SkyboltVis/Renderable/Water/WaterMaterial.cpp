/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WaterMaterial.h"
#include "SkyboltVis/Renderable/Water/WaterStateSet.h"
#include "SkyboltVis/Renderable/Water/WaveHeightTextureGenerator.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGenerator.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGeneratorStateSets.h"

#include <osg/Group>
#include <osg/Switch>
#include <boost/log/trivial.hpp>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Texture2D> createRenderTextureRgb16F(int width, int height)
{
#ifdef USE_DELL_XPS_RTT_FIX
	// Fixes bug on integrated Dell xps 13 graphics where textures attached to frame buffer render black if image is not allocated first
	osg::Image* outputImage = new osg::Image();
	outputImage->allocateImage(width, height, 1, GL_RGB16F_ARB, GL_UNSIGNED_BYTE);

	osg::Texture2D* texture = new osg::Texture2D(outputImage);
#else
	osg::Texture2D* texture = new osg::Texture2D();
#endif
	texture->setTextureWidth(width);
	texture->setTextureHeight(height);
	texture->setInternalFormat(GL_RGB16F_ARB);
	return texture;
}

static osg::ref_ptr<osg::Texture2D> createNormalTexture(int width, int height)
{
	// Use a 16 bit float texture because an 8 bit RGB texture causes aliasing artifacts due to insufficient precision
	osg::ref_ptr<osg::Texture2D> texture = createRenderTextureRgb16F(width, height);

	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

	return texture;
}

osg::Texture2D* createFoamMaskTexture(int width, int height)
{
	// Create the texture object and set the image
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE);
	memset(image->data(), 0, width * height);
	osg::Texture2D* texture = new osg::Texture2D(image);
	texture->setTextureWidth(width);
	texture->setTextureHeight(height);
	texture->setInternalFormat(GL_RED);
	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

	return texture;
}

struct WaveFoamMaskGeneratorConfig
{
	osg::ref_ptr<osg::Program> program;
	osg::ref_ptr<osg::Texture2D> waveVectorDisplacementTexture;
	osg::Vec2f textureWorldSize;
	osg::Vec2i textureSizePixels;
};

//! Generates a foam mask texture from a wave vector displacement texture.
//! There are two output buffers which are ping-ponged so that the output from the previous
//! frame can be used as an input to the next. In this way, the foam can be made to fade out
//! slowly over time.
class WaveFoamMaskGenerator : public osg::Group
{
public:
	WaveFoamMaskGenerator(const WaveFoamMaskGeneratorConfig& config) :
		mSwitch(new osg::Switch()),
		mIndex(0)
	{
		const float jacobianLambda = 1.0f;
		const float generateMipMaps = true;

		for (int i = 0; i < 2; ++i)
		{
			mOutputTexture[i] = createFoamMaskTexture(config.textureSizePixels.x(), config.textureSizePixels.y());
		}

		for (int i = 0; i < 2; ++i)
		{
			const int otherIndex = (i == 0);
			mGenerator[i] = new GpuTextureGenerator(mOutputTexture[i], createWaveFoamMaskGeneratorStateSet(config.program, config.waveVectorDisplacementTexture, mOutputTexture[otherIndex], config.textureWorldSize, jacobianLambda), generateMipMaps);
			mSwitch->addChild(mGenerator[i], i == 0);
		}

		addChild(mSwitch);

		mFoamMaskSubtractionAmountUniform = new osg::Uniform("foamMaskSubtractionAmount", 0.0f);
		mSwitch->getOrCreateStateSet()->addUniform(mFoamMaskSubtractionAmountUniform);
	}

	~WaveFoamMaskGenerator()
	{
		mSwitch->getParent(0)->removeChild(mSwitch);
	}

	void swapBuffers()
	{
		mIndex = (mIndex + 1) % 2;
		mSwitch->setValue(0, mIndex);
		mSwitch->setValue(1, !mIndex);
	}

	void advanceTime(double timeSeconds)
	{
		// Update subtraction amount for previous frame's foam mask when mixing with the current frame.
		// This makes old foam slowly fade out over time.
		const float totalFadeTime = 30.0f;
		float amount = std::min(1.0, std::abs(timeSeconds - mPrevTimeSeconds)) / totalFadeTime;
		// Don't fade this frame if the amount is less than the minimum value that has any effect in 8 bit textures.
		if (amount >= 0.005)
		{
			mPrevTimeSeconds = timeSeconds;
		}
		else
		{
			amount = 0;
		}
		mFoamMaskSubtractionAmountUniform->set(amount);

		mGenerator[mIndex]->requestRegenerate();
	}

	osg::ref_ptr<osg::Texture2D> getOutputTexture() const
	{
		return mOutputTexture[mIndex];
	}

private:
	osg::ref_ptr<osg::Switch> mSwitch;
	// Two outputs so we can ping-pong between them.
	// This is required for feeding the output of the previous into the input of the next
	// so that the foam can persist over time.
	osg::ref_ptr<GpuTextureGenerator> mGenerator[2];
	osg::ref_ptr<osg::Texture2D> mOutputTexture[2];
	int mIndex;

	osg::ref_ptr<osg::Uniform> mFoamMaskSubtractionAmountUniform;
	double mPrevTimeSeconds = 0;
};

WaterMaterial::WaterMaterial(const WaterMaterialConfig& config)
{
	WaterStateSetConfig stateSetConfig;

	mWaveHeightTextureGenerator = config.factory->create();
	const int cascadeCount = mWaveHeightTextureGenerator->getTextureCount();

	for (int i = 0; i < cascadeCount; ++i)
	{
		WaterStateSetConfig::WaveTextureSet waveTextureSet;

		float textureWorldSize = mWaveHeightTextureGenerator->getTextureWorldSize(i);
		waveTextureSet.waveHeightMapWorldSizes = textureWorldSize;

		waveTextureSet.waveHeight = mWaveHeightTextureGenerator->getTexture(i);
		waveTextureSet.waveHeight->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		waveTextureSet.waveHeight->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

		int width = waveTextureSet.waveHeight->getImage(0)->s();
		int height = waveTextureSet.waveHeight->getImage(0)->t();

		osg::Vec2f textureWorldSize2d(textureWorldSize, textureWorldSize);
		const bool generateMipMaps = true;
		waveTextureSet.waveNormal = createNormalTexture(width, height);
		osg::ref_ptr<GpuTextureGenerator> generator = new GpuTextureGenerator(waveTextureSet.waveNormal, createVectorDisplacementToNormalMapStateSet(config.programs->getRequiredProgram("vectorDisplacementToNormal"), waveTextureSet.waveHeight, textureWorldSize2d), generateMipMaps);
		addChild(generator);
		mWaterSurfaceGpuTextureGenerators.push_back(generator);

		WaveFoamMaskGeneratorConfig generatorConfig;
		generatorConfig.program = config.programs->getRequiredProgram("waveFoamMaskGenerator");
		generatorConfig.textureSizePixels = osg::Vec2i(width, height);
		generatorConfig.textureWorldSize = textureWorldSize2d;
		generatorConfig.waveVectorDisplacementTexture = waveTextureSet.waveHeight;
		
		osg::ref_ptr<WaveFoamMaskGenerator> waveFoamMaskGenerator = new WaveFoamMaskGenerator(generatorConfig);
		mWaveFoamMaskGenerators.push_back(waveFoamMaskGenerator);
		addChild(waveFoamMaskGenerator);
		waveTextureSet.waveFoamMask = waveFoamMaskGenerator->getOutputTexture();

		stateSetConfig.waveTextureSets.push_back(waveTextureSet);
	}

	setStateSet(new WaterStateSet(stateSetConfig));
}

void WaterMaterial::setWaveHeight(float height)
{
	mWaveHeightTextureGenerator->setWaveHeight(height);
}

float WaterMaterial::getWaveHeight() const
{
	return mWaveHeightTextureGenerator->getWaveHeight();
}

void WaterMaterial::setWindVelocityHeading(float heading)
{
	mWaveHeightTextureGenerator->setWindVelocityHeading(heading);
}

float WaterMaterial::getWindVelocityHeading() const
{
	return mWaveHeightTextureGenerator->getWindVelocityHeading();
}

void WaterMaterial::setWakes(const std::vector<Wake>& wakes)
{
	static_cast<WaterStateSet*>(getStateSet())->setWakes(wakes);
}

void WaterMaterial::update(double timeSeconds)
{
	double loopPeriod = 1000; // TODO: Seamless looping
	double loopedTimeSeconds = std::fmod(timeSeconds, loopPeriod);
		
	if (mWaveHeightTextureGenerator->generate(loopedTimeSeconds))
	{
		for (const auto& generator : mWaterSurfaceGpuTextureGenerators)
		{
			generator->requestRegenerate();
		}

		for (int i = 0; i < mWaveFoamMaskGenerators.size(); ++i)
		{
			mWaveFoamMaskGenerators[i]->advanceTime(loopedTimeSeconds);
			static_cast<WaterStateSet*>(getStateSet())->setFoamTexture(i, mWaveFoamMaskGenerators[i]->getOutputTexture());
			mWaveFoamMaskGenerators[i]->swapBuffers();
		}
	}
}

int WaterMaterial::getCascadeCount() const
{
	return mWaveHeightTextureGenerator->getTextureCount();
}

osg::ref_ptr<osg::Texture2D> WaterMaterial::getHeightTexture(int cascadeIndex) const
{
	return mWaveHeightTextureGenerator->getTexture(cascadeIndex);
}

osg::ref_ptr<osg::Texture2D> WaterMaterial::getNormalTexture(int cascadeIndex) const
{
	return osg::static_pointer_cast<osg::Texture2D>(mWaterSurfaceGpuTextureGenerators[cascadeIndex]->getOutputTextures().front());
}

osg::ref_ptr<osg::Texture2D> WaterMaterial::getFoamMaskTexture(int cascadeIndex) const
{
	return mWaveFoamMaskGenerators[cascadeIndex]->getOutputTexture();
}

sim::OceanSurfaceSamplerPtr WaterMaterial::getSurfaceSampler() const
{
	return mWaveHeightTextureGenerator->getSurfaceSampler();
}

} // namespace vis
} // namespace skybolt
