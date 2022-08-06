/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "WaterMaterial.h"
#include "SkyboltVis/Renderable/Water/WaterStateSet.h"
#include "SkyboltVis/Renderable/Water/WaveHeightTextureGenerator.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGenerator.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGeneratorStateSets.h"

#include <osg/Group>
#include <osg/Switch>

namespace skybolt {
namespace vis {

static osg::ref_ptr<osg::Texture2D> createNormalTexture(int width, int height)
{
	osg::ref_ptr<osg::Texture2D> texture = createRenderTexture(width, height);
	texture->setInternalFormat(GL_RGBA);
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

//! @param textureScale is textureSizeInWorldSpace / maxHeightInWorldSpace
osg::StateSet* createHeightToNormalMapStateSet(osg::ref_ptr<osg::Program> program, osg::Texture2D* heightTexture, const osg::Vec2f& textureScale)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(0, heightTexture, osg::StateAttribute::ON);

	const osg::Image& image = *heightTexture->getImage(0);
	osg::Vec2f texelSize(1.0f / (float)image.s(), 1.0f / (float)image.t());
	osg::Vec2f texelScale(textureScale.x() * texelSize.x(), textureScale.y() * texelSize.y());
	stateSet->addUniform(new osg::Uniform("texelSizeInTextureSpace", texelSize));
	stateSet->addUniform(new osg::Uniform("texelScale", texelScale));

	return stateSet;
}

class CascadedWaveHeightTextureGenerator
{
public:
	CascadedWaveHeightTextureGenerator(const WaveHeightTextureGeneratorFactory& factory, float smallestTextureWorldSize)
	{
		for (int i = 0; i < numCascades; ++i)
		{
			float scale = (i == 0) ? 1 : 5;
			float worldSize = smallestTextureWorldSize * scale;
			glm::vec2 normalizedFrequencyRange = glm::vec2(0.0, 1);

			// If this is not the last cascade, cut off the lower part of the frequency range so that the frequencies are not repeated in other cascades
			if (i < numCascades - 1)
			{
				normalizedFrequencyRange = glm::vec2(0.2, 1);
			}

			mCascades[i] = factory.create(worldSize, normalizedFrequencyRange);
			mWorldSizes[i] = worldSize;
		}
	}

	bool update(double time)
	{
		bool updated = false;
		for (int i = 0; i < numCascades; ++i)
		{
			if (mCascades[i]->generate(time))
			{
				updated = true;
			}
		}
		return updated;
	}

	float getWorldSize(int i) const
	{
		return mWorldSizes[i];
	}

	osg::ref_ptr<osg::Texture2D> getTexture(int cascade) const
	{
		return mCascades[cascade]->getTexture();
	}

	float getWaveHeight() const
	{
		return mCascades[0]->getWaveHeight();
	}

	void setWaveHeight(float height)
	{
		for (const auto& cascade : mCascades)
		{
			cascade->setWaveHeight(height);
		}
	}

	static const int numCascades = 1;

private:
	std::unique_ptr<WaveHeightTextureGenerator> mCascades[numCascades];
	float mWorldSizes[numCascades];
};

static_assert(WaterStateSetConfig::waveTextureCount == CascadedWaveHeightTextureGenerator::numCascades, "Number of wave image cascades should match number of textures");

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
			const int otherIndex = !i;
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

	float smallestWaveHeightMapWorldSize = 500.0f; // FIXME: To avoid texture wrapping issues, Scene::mWrappedNoisePeriod divided by this should have no remainder
	mWaveHeightTextureGenerator.reset(new CascadedWaveHeightTextureGenerator(*config.factory, smallestWaveHeightMapWorldSize));

	for (int i = 0; i < CascadedWaveHeightTextureGenerator::numCascades; ++i)
	{
		stateSetConfig.waveHeightMapWorldSizes[i] = mWaveHeightTextureGenerator->getWorldSize(i);

		stateSetConfig.waveHeightTexture[i] = mWaveHeightTextureGenerator->getTexture(i);
		stateSetConfig.waveHeightTexture[i]->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		stateSetConfig.waveHeightTexture[i]->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

		int width = stateSetConfig.waveHeightTexture[i]->getImage(0)->s();
		int height = stateSetConfig.waveHeightTexture[i]->getImage(0)->t();

		osg::Vec2f textureWorldSize(mWaveHeightTextureGenerator->getWorldSize(i), mWaveHeightTextureGenerator->getWorldSize(i));
		const bool generateMipMaps = true;
		stateSetConfig.waveNormalTexture[i] = createNormalTexture(width, height);
		osg::ref_ptr<GpuTextureGenerator> generator = new GpuTextureGenerator(stateSetConfig.waveNormalTexture[i], createVectorDisplacementToNormalMapStateSet(config.programs->getRequiredProgram("vectorDisplacementToNormal"), stateSetConfig.waveHeightTexture[i], textureWorldSize), generateMipMaps);
		addChild(generator);
		mWaterSurfaceGpuTextureGenerators.push_back(generator);

		WaveFoamMaskGeneratorConfig generatorConfig;
		generatorConfig.program = config.programs->getRequiredProgram("waveFoamMaskGenerator");
		generatorConfig.textureSizePixels = osg::Vec2i(width, height);
		generatorConfig.textureWorldSize = textureWorldSize;
		generatorConfig.waveVectorDisplacementTexture = stateSetConfig.waveHeightTexture[i];
		mWaveFoamMaskGenerator[i] = new WaveFoamMaskGenerator(generatorConfig);
		addChild(mWaveFoamMaskGenerator[i]);
		stateSetConfig.waveFoamMaskTexture[i] = mWaveFoamMaskGenerator[i]->getOutputTexture();
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

void WaterMaterial::setWakes(const std::vector<Wake>& wakes)
{
	static_cast<WaterStateSet*>(getStateSet())->setWakes(wakes);
}

void WaterMaterial::update(double timeSeconds)
{
	double loopPeriod = 1000; // TODO: Seamless looping
	double loopedTimeSeconds = std::fmod(timeSeconds, loopPeriod);
		
	if (mWaveHeightTextureGenerator->update(loopedTimeSeconds))
	{
		for (const auto& generator : mWaterSurfaceGpuTextureGenerators)
		{
			generator->requestRegenerate();
		}

		for (int i = 0; i < CascadedWaveHeightTextureGenerator::numCascades; ++i)
		{
			mWaveFoamMaskGenerator[i]->advanceTime(loopedTimeSeconds);
			static_cast<WaterStateSet*>(getStateSet())->setFoamTexture(i, mWaveFoamMaskGenerator[i]->getOutputTexture());
			mWaveFoamMaskGenerator[i]->swapBuffers();
		}
	}
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
	return mWaveFoamMaskGenerator[cascadeIndex]->getOutputTexture();
}

} // namespace vis
} // namespace skybolt
