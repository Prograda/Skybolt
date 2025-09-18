/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltVis/Renderable/Water/WaveHeightTextureGenerator.h>
#include "WaveSpectrumWindow.h"
#include "FftOceanGenerator.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace skybolt {
namespace vis {

struct FftOceanWaveHeightTextureGeneratorConfig
{
	float textureWorldSize;
	int textureSizePixels;
	WaveSpectrumWindow waveSpectrumWindow;
};

class FftOceanWaveHeightTextureGenerator : public WaveHeightTextureGenerator
{
public:
	FftOceanWaveHeightTextureGenerator(const FftOceanWaveHeightTextureGeneratorConfig& config) :
		mWorldSize(config.textureWorldSize),
		mWaveHeight(0.5)
	{
		mWindVelocity = calcWindVelocity();

		mGeneratorResult = std::vector<glm::vec3>(config.textureSizePixels * config.textureSizePixels, glm::vec3(0,0,0));

		mGenerator.reset(new FftOceanGenerator([&] {
			FftOceanGeneratorConfig c;
			c.gravity = mGravity;
			c.seed = 0;
			c.textureSizePixels = config.textureSizePixels;
			c.textureWorldSize = config.textureWorldSize;
			c.windVelocity = mWindVelocity;
			c.waveSpectrumWindow = config.waveSpectrumWindow;
			return c;
			}()));

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

				if (mWindVelocityChanged)
				{
					mGenerator->setWindVelocity(mWindVelocity);
					mWindVelocityChanged = false;
				}

				mGenerator->calculate(mRequestTime, span<glm::vec3>{mGeneratorResult.data(), mGeneratorResult.size()});
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

	void setWaveHeight(float height) override
	{
		if (mWaveHeight != height)
		{
			mWaveHeight = height;
			mWindVelocity = calcWindVelocity();
			mWindVelocityChanged = true;
		}
	}

	float getWindVelocityHeading() const override { return mWindVelocityHeading; }

	void setWindVelocityHeading(float heading) override
	{
		if (mWindVelocityHeading != heading)
		{
			mWindVelocityHeading = heading;
			mWindVelocity = calcWindVelocity();
			mWindVelocityChanged = true;
		}
	}

	int getTextureCount() const { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const
	{
		assert(index == 0);
		return mTexture;
	}

	float getTextureWorldSize(int index) const { return mWorldSize; }

private:
	glm::vec2 calcWindVelocity() const {
		float windSpeed = FftOceanGenerator::calcWindSpeedFromMaxWaveHeight(mWaveHeight, mGravity);
		return math::vec2Rotate(glm::vec2(windSpeed, 0), mWindVelocityHeading);
	}

private:
	const float mGravity = 9.8f;
	std::unique_ptr<FftOceanGenerator> mGenerator;
	osg::ref_ptr<osg::Texture2D> mTexture;
	float mWorldSize;
	float mWaveHeight;
	float mWindVelocityHeading = 0;

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

	std::atomic<glm::vec2> mWindVelocity;
	std::atomic_bool mWindVelocityChanged = false;
};

} // namespace vis
} // namespace skybolt
