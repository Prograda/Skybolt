/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
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

	osg::ref_ptr<osg::Texture2D> getTexture() const override
	{
		return mTexture;
	}

private:
	osg::ref_ptr<osg::Texture2D> mTexture;
	bool mNewResult = true;
};

class SimpleWaveHeightTextureGeneratorFactory : public WaveHeightTextureGeneratorFactory
{
public:
	std::unique_ptr<WaveHeightTextureGenerator> create(float textureWorldSize, const glm::vec2& normalizedFrequencyRange) const override
	{
		return std::make_unique<SimpleWaveHeightTextureGenerator>();
	}
};

} // namespace vis
} // namespace skybolt
