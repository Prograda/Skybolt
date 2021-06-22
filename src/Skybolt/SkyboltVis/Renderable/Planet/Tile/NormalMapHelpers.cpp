/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NormalMapHelpers.h"
#include <algorithm>
#include <assert.h>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Image> createNormalmapFromHeightmap(const osg::Image& heightmap, const osg::Vec2f& texelWorldSize)
{
	assert(heightmap.getInternalTextureFormat() == GL_R16);

	const int width = heightmap.s();
	const int height = heightmap.t();

	osg::Image* image = new osg::Image;
	image->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	image->setInternalTextureFormat(GL_RGB8);

	unsigned char* p = image->data();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int px = std::min(x, width - 2);
			int py = std::min(y, height - 2);

			uint16_t h00 = *(const uint16_t*)&heightmap.data()[2 * (px + heightmap.s() * py)];
			uint16_t h10 = *(const uint16_t*)&heightmap.data()[2 * ((px + 1) + heightmap.s() * py)];
			uint16_t h01 = *(const uint16_t*)&heightmap.data()[2 * (px + heightmap.s() * (py + 1))];
			uint16_t h11 = *(const uint16_t*)&heightmap.data()[2 * ((px + 1) + heightmap.s() * (py + 1))];

			float dhx = 0.5f * ((h10 + h11) - (h00 + h01));
			float dhy = 0.5f * ((h01 + h11) - (h00 + h10));

			osg::Vec3f normal = osg::Vec3f(texelWorldSize.x(), 0, dhx) ^ osg::Vec3f(0, texelWorldSize.y(), dhy);
			normal.normalize();

			*p++ = normal.x() * 128.0f + 128.0f;
			*p++ = normal.y() * 128.0f + 128.0f;
			p++;
		}
	}
	return image;
}

} // namespace vis
} // namespace skybolt
