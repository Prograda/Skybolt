/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CloudNoiseTextureGenerator.h"
#include "SkyboltVis/Renderable/Clouds/ThirdParty/TileableVolumeNoise.h"

#include <algorithm>
#include <functional>
#include <glm/glm.hpp>

namespace skybolt {
namespace vis {

static float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

static int luminanceToUnsignedByte(float f)
{
	return std::min(255, int(f * 256.f));
}

typedef std::function<glm::vec4(const glm::vec3&)> ColorAtPositionGetter;
static void fillVolumeTexture(osg::Image& image, ColorAtPositionGetter getter, int channelCount = 1)
{
	unsigned char* p = image.data();

	for (int z = 0; z < image.r(); ++z)
	{
		for (int y = 0; y < image.t(); ++y)
		{
			for (int x = 0; x < image.s(); ++x)
			{
				glm::vec3 pos = glm::vec3(x, y, z) / float(image.s());

				glm::vec4 c = getter(pos);
				for (int i = 0; i < channelCount; ++i)
				{
					*p++ = luminanceToUnsignedByte(c[i]);
				}
			}
		}
	}
}

float createWorleyFbm(const glm::vec3& pos)
{
	float worleyNoise = 0;
	float frequency = 8.0;
	float amplitude = 0.5;
	for (int i = 0; i < 4; ++i)
	{
		worleyNoise += amplitude * (1.0 - Tileable3dNoise::WorleyNoise(pos, frequency));
		frequency *= 1.8f;
		amplitude *= 0.6f;
	}

	return glm::clamp(worleyNoise - 0.2f, 0.0f, 1.0f);
}

float createPerlinWorley(const glm::vec3& pos, const PerlinWorleyConfig& config)
{
	float worleyNoise = createWorleyFbm(pos);

	float perlinNoise = Tileable3dNoise::PerlinNoise(pos, config.frequency, config.octaves);
	float perlWorlNoise = remap(perlinNoise, 0.5*(1.0 - worleyNoise), 1.0f, 0.0f, 1.0f);
	perlWorlNoise = glm::clamp(perlWorlNoise, 0.0f, 1.0f);
	return perlWorlNoise;
}

osg::ref_ptr<osg::Image> createPerlinWorleyTexture(const PerlinWorleyConfig& config)
{
	osg::Image* image = new osg::Image;
	image->allocateImage(config.width, config.width, config.width, GL_LUMINANCE, GL_UNSIGNED_BYTE);

	fillVolumeTexture(*image, [=](const glm::vec3& pos) {
		float c = 1.0 - createWorleyFbm(pos);
		//float c = createPerlinWorley(pos, config); // Use to modulate output by perlin
		return glm::vec4(c, c, c, 1.0);
	});

	return image;
}

} // namespace vis
} // namespace skybolt