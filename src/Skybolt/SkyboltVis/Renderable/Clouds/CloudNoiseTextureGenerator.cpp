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
static void fillVolumeTexture(osg::Image& image, ColorAtPositionGetter getter)
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
				*p++ = luminanceToUnsignedByte(c.r);
				*p++ = luminanceToUnsignedByte(c.g);
				*p++ = luminanceToUnsignedByte(c.b);
				*p++ = luminanceToUnsignedByte(c.a);
			}
		}
	}
}

float createPerlinWorley(const glm::vec3& pos, const PerlinWorleyConfig& config)
{
	float perlinNoise = Tileable3dNoise::PerlinNoise(pos, config.frequency, config.octaves);
	float worleyNoise0 = 1.0 - Tileable3dNoise::WorleyNoise(pos, 16.0f);
	float worleyNoise1 = 1.0 - Tileable3dNoise::WorleyNoise(pos, 32.0f);
	float worleyNoise2 = 1.0 - Tileable3dNoise::WorleyNoise(pos, 64.0f);

	float worleyFBM = worleyNoise0 * 0.5f + worleyNoise1 * 0.25f + worleyNoise2 * 0.25f;
	worleyFBM = glm::clamp(worleyFBM, 0.0f, 1.0f);

	float perlWorlNoise = remap(perlinNoise, 0.999*(1.0 - worleyFBM), 1.0f, 0.0f, 1.0f);
	perlWorlNoise = glm::clamp(perlWorlNoise, 0.0f, 1.0f);
	return perlWorlNoise;
}

osg::ref_ptr<osg::Image> createPerlinWorleyTexture(const PerlinWorleyConfig& config)
{
	osg::Image* image = new osg::Image;
	image->allocateImage(config.width, config.width, config.width, GL_RGBA, GL_UNSIGNED_BYTE);

	fillVolumeTexture(*image, [=](const glm::vec3& pos) {
		return glm::vec4(1.0 - createPerlinWorley(pos, config), 1.0 - createPerlinWorley(glm::vec3(pos.y, pos.z, pos.x), config), 0.0, 0.0);
	});

	return image;
}

} // namespace vis
} // namespace skybolt