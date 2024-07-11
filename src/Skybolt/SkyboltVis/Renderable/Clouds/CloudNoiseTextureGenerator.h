/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Image>

namespace skybolt {
namespace vis {

struct FbmConfig
{
	float frequency = 8.0;
	float amplitude = 0.5;

	int octaveCount = 4;
	float lacunarity = 2.0f;
	float gain = 0.6f;

	bool invert = false;
};

struct PerlinWorleyConfig
{
	int width = 128;
	int perlinOctaves = 6;
	float perlinFrequency = 8.0f;
	FbmConfig worley;
};

osg::ref_ptr<osg::Image> createPerlinWorleyTexture2d(const PerlinWorleyConfig& config);
osg::ref_ptr<osg::Image> createPerlinWorleyTexture3d(const PerlinWorleyConfig& config);

} // namespace vis
} // namespace skybolt