/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "WaveHeightTextureGenerator.h"

namespace skybolt {
namespace vis {

class SimpleWaveHeightTextureGenerator : public WaveHeightTextureGenerator
{
public:
	SimpleWaveHeightTextureGenerator();

	bool generate(double time)
	{
		bool newResult = mNewResult;
		mNewResult = false;
		return newResult;
	}

	float getWaveHeight() const override { return 0.0f; }
	void setWaveHeight(float height) override {};

	float getWindVelocityHeading() const override { return 0.0f; }
	void setWindVelocityHeading(float heading) override {};

	int getTextureCount() const override { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const override
	{
		return mTexture;
	}

	float getTextureWorldSize(int index) const { return 100; } // Precise size doesn't matter as the ocean surface is flat

private:
	osg::ref_ptr<osg::Texture2D> mTexture;
	bool mNewResult = true;
};

class SimpleWaveHeightTextureGeneratorFactory : public WaveHeightTextureGeneratorFactory
{
public:
	std::unique_ptr<WaveHeightTextureGenerator> create() const override
	{
		return std::make_unique<SimpleWaveHeightTextureGenerator>();
	}
};

} // namespace vis
} // namespace skybolt
