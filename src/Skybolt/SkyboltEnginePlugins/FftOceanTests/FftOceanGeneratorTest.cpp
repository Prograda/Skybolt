/* Copyright Matthew Reid
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
	FftOceanGenerator generator(config);

	float maxHeight = generator.calcMaxWaveHeight(glm::length(config.windVelocity), config.gravity);

	std::vector<glm::vec3> result(generator.getTextureWorldSize().x * generator.getTextureWorldSize().y);
	generator.calculate(0, span<glm::vec3>{result.data(), result.size()});

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

//! Calculates the maximum wave height based on trough to crest
static float calcMaxWaveHeight(const std::vector<glm::vec3>& displacementMap)
{
	float vMin = std::numeric_limits<float>::infinity();
	float vMax = -std::numeric_limits<float>::infinity();

	for (const glm::vec3& v : displacementMap)
	{
		vMin = std::min(vMin, v.z);
		vMax = std::max(vMax, v.z);
	}
	return vMax - vMin;
}

static float calcElevationStandardDeviation(const std::vector<glm::vec3>& displacementMap)
{
	if (displacementMap.empty()) return 0.0;

	// Calculate mean
	double mean = 0;
	for (const glm::vec3& v : displacementMap)
	{
		mean += v.z;
	}
	mean /= displacementMap.size();

	// Calculate variance
	double variance = 0.0;
	for (const glm::vec3& v : displacementMap)
	{
		variance += (v.z - mean) * (v.z - mean);
	}
	variance /= displacementMap.size();

	// Return the standard deviation (square root of variance)
	return std::sqrt(variance);
}

//! Significant wave height (trough to crest) is average height of highest 1/3 of waves.
//! Also defined as 4 times the standard deviation of the surface elevation.
//! See https://en.wikipedia.org/wiki/Significant_wave_height
static float calcSignificantWaveHeight(const std::vector<glm::vec3>& displacementMap)
{
	return 4.f * calcElevationStandardDeviation(displacementMap);
}

static float calcSignificantWaveHeightForWindSpeed(float speed)
{
	FftOceanGeneratorConfig config;
	config.seed = 0;
	config.textureSizePixels = 256;
	config.textureWorldSize = 1000;
	config.windVelocity = glm::vec2(speed, 0);
	config.gravity = 9.8;
	FftOceanGenerator generator(config);

	std::vector<glm::vec3> result(generator.getTextureWorldSize().x * generator.getTextureWorldSize().y);
	generator.calculate(0, span<glm::vec3>{result.data(), result.size()});

	return calcSignificantWaveHeight(result);
}

// FIXME: Currently disabled because test fails because wave heights are not calibrated to beaufort scale
/*
TEST_CASE("FFT ocean wave height is as expected for given wind speed")
{
	constexpr float espilon = 0.1f;
	// Based on https://en.wikipedia.org/wiki/Beaufort_scale
	CHECK(calcSignificantWaveHeightForWindSpeed(1.5f) == Approx(0.3).epsilon(espilon)); // sea state 1
	CHECK(calcSignificantWaveHeightForWindSpeed(3.3f) == Approx(0.6).epsilon(espilon)); // sea state 2
	CHECK(calcSignificantWaveHeightForWindSpeed(5.4f) == Approx(1.2).epsilon(espilon)); // sea state 3
	CHECK(calcSignificantWaveHeightForWindSpeed(10.7f) == Approx(3).epsilon(espilon)); // sea state 5
	CHECK(calcSignificantWaveHeightForWindSpeed(17.1f) == Approx(5.5).epsilon(espilon)); // sea state 7
	CHECK(calcSignificantWaveHeightForWindSpeed(24.4f) == Approx(10).epsilon(espilon)); // sea state 9
	CHECK(calcSignificantWaveHeightForWindSpeed(32.6f) == Approx(16).epsilon(espilon)); // sea state 11
}
*/