/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <FftOcean/FftOceanGenerator.h>

#include <osgDB/WriteFile>

using namespace skybolt::vis;

TEST_CASE("Generate FFT ocean texture")
{
	FftOceanGeneratorConfig config;
	config.seed = 0;
	config.textureSizePixels = 256;
	config.textureWorldSize = 1000;
	config.windVelocity = glm::vec2(10, 0);
	config.gravity = 9.8;
	config.normalizedFrequencyRange = glm::vec2(0, 1);
	FftOceanGenerator generator(config);

	float maxHeight = generator.calcMaxWaveHeight(glm::length(config.windVelocity), config.gravity);

	std::vector<glm::vec3> result;
	generator.calculate(0, result);

	float vMax = 0;

	size_t elementCount = result.size();
	std::vector<unsigned char> resultChar(result.size() * 3);
	for (size_t i = 0; i < elementCount * 3; ++i)
	{
		float v = reinterpret_cast<float*>(result.data())[i];
		resultChar[i] = 128.f + 128.f * v / maxHeight;

		vMax = std::max(vMax, v);
	}

	CHECK(vMax > maxHeight * 0.4f);
	CHECK(vMax < maxHeight * 2.0f);

	if (0)
	{
		osg::Image* image = new osg::Image();
		image->allocateImage(config.textureSizePixels, config.textureSizePixels, 1, GL_RGB, GL_BYTE);
		memcpy(image->data(), resultChar.data(), elementCount * 3);
		osgDB::writeImageFile(*image, "C:/Users/Public/test.tga");
	}
}
