/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltVis/Renderable/Water/WaveHeightTextureGenerator.h>
#include "FftOceanGenerator.h"

#include <mutex>
#include <thread>

namespace skybolt {
namespace vis {

class FftOceanWaveHeightTextureGenerator : public WaveHeightTextureGenerator
{
public:
	FftOceanWaveHeightTextureGenerator(float textureWorldSize, const glm::vec2& normalizedFrequencyRange) :
		mWorldSize(textureWorldSize),
		mWaveHeight(0.5)
	{
		mWindSpeed = FftOceanGenerator::calcWindSpeedFromMaxWaveHeight(mWaveHeight, mGravity);

		FftOceanGeneratorConfig config;
		config.gravity = mGravity;
		config.seed = 0;
		config.textureSizePixels = 512;
		config.textureWorldSize = textureWorldSize;
		config.windVelocity = glm::vec2(mWindSpeed.load(), 0);
		config.normalizedFrequencyRange = normalizedFrequencyRange;

		mGeneratorResult = std::vector<glm::vec3>(config.textureSizePixels * config.textureSizePixels, glm::vec3(0,0,0));

		mGenerator.reset(new FftOceanGenerator(config));

		osg::Image* image = new osg::Image();
		image->setDataVariance(osg::Image::DYNAMIC);
		image->allocateImage(config.textureSizePixels, config.textureSizePixels, 1, GL_RGB, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGB32F_ARB);

		mTexture = new osg::Texture2D(image);
		mTexture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR_MIPMAP_LINEAR);
		mTexture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);

		mGeneratorThread = std::thread([this] {runGeneratorThread(); });
		{
			std::lock_guard<std::mutex> lock(mGeneratorRequestMutex);
			mGeneratorRequest = true;
		}
		mGeneratorRequestCV.notify_one();

		generate(0);
	}

	~FftOceanWaveHeightTextureGenerator()
	{
		mTerminateGeneratorThread = true;
		mGeneratorRequestCV.notify_one();
		mGeneratorThread.join();
	}

	bool generate(double time) override
	{
		if (time == mResultTime)
		{
			return false;
		}

		if (mGeneratorHasResult)
		{
			osg::Image* image = mTexture->getImage();
			{
				// Copy image from generator thread to texture
				std::lock_guard<std::mutex> lock(mGeneratorResultMutex);
				memcpy(image->data(), mGeneratorResult.data(), mGeneratorResult.size() * sizeof(float) * 3);
				mResultTime = mRequestTime;

				// Request a new image to be generated
				mGeneratorRequest = true;
				mRequestTime = time;
			}
			mGeneratorRequestCV.notify_one();
			image->dirty();
			return true;
		}
		return false;
	}

	void runGeneratorThread()
	{
		while (!mTerminateGeneratorThread)
		{
			// Wait for generation request
			{
				std::unique_lock<std::mutex> lock(mGeneratorRequestMutex);
				mGeneratorRequestCV.wait(lock, [this] {return mGeneratorRequest || mTerminateGeneratorThread; });
			}

			if (mTerminateGeneratorThread)
			{
				return;
			}

			{
				std::lock_guard<std::mutex> lock(mGeneratorResultMutex);

				if (mWindSpeedChanged)
				{
					mGenerator->setWindSpeed(mWindSpeed);
					mWindSpeedChanged = false;
				}

				mGenerator->calculate(mRequestTime, mGeneratorResult);
				mGeneratorHasResult = true;
				mGeneratorRequest = false;
			}
		}
	}

	float getWorldSize() const
	{
		return mWorldSize;
	}

	float getWaveHeight() const override { return mWaveHeight; }

	void setWaveHeight(float height)
	{
		if (mWaveHeight != height)
		{
			mWaveHeight = height;
			mWindSpeed = FftOceanGenerator::calcWindSpeedFromMaxWaveHeight(height, mGravity);
			mWindSpeedChanged = true;
		}
	}

	osg::ref_ptr<osg::Texture2D> getTexture() const
	{
		return mTexture;
	}

private:
	const float mGravity = 9.8f;
	std::unique_ptr<FftOceanGenerator> mGenerator;
	osg::ref_ptr<osg::Texture2D> mTexture;
	float mWorldSize;
	float mWaveHeight;

	std::thread mGeneratorThread;
	std::atomic_bool mTerminateGeneratorThread = false;

	std::vector<glm::vec3> mGeneratorResult;
	std::mutex mGeneratorResultMutex;
	std::atomic_bool mGeneratorHasResult = false;
	double mResultTime = std::numeric_limits<double>::infinity();

	std::mutex mGeneratorRequestMutex;
	std::condition_variable mGeneratorRequestCV;
	double mRequestTime = 0;
	bool mGeneratorRequest = false;

	std::atomic<float> mWindSpeed;
	std::atomic_bool mWindSpeedChanged = false;
};

} // namespace vis
} // namespace skybolt
