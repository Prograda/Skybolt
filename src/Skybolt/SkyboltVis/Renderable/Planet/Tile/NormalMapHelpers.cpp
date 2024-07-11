/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NormalMapHelpers.h"
#include <osg/Texture> // included for GL_R16
#include <algorithm>
#include <assert.h>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Image> createNormalMapFromHeightMap(const osg::Image& heightmap, const HeightMapElevationRerange& rerange, const osg::Vec2f& texelWorldSize, int filterWidth)
{
	assert(heightmap.getInternalTextureFormat() == GL_R16);
	const int width = heightmap.s();
	const int height = heightmap.t();

	osg::Image* image = new osg::Image;
	image->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	image->setInternalTextureFormat(GL_RGB8);

	unsigned char* p = image->data();
	const uint16_t* src = reinterpret_cast<const uint16_t*>(heightmap.data());

	float filterWidthF = filterWidth;
	int lowerOffset = -(filterWidth / 2);
	int upperOffset = lowerOffset + filterWidth;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int x0 = std::clamp(x + lowerOffset, 0, width-1-filterWidth);
			int y0 = std::clamp(y + lowerOffset, 0, height-1-filterWidth);
			int x1 = std::clamp(x + upperOffset, filterWidth, width-1);
			int y1 = std::clamp(y + upperOffset, filterWidth, height-1);

			uint16_t h00 = src[x0 + width * y0];
			uint16_t h10 = src[x1 + width * y0];
			uint16_t h01 = src[x0 + width * y1];
			uint16_t h11 = src[x1 + width * y1];

			const float elevationScale = rerange.x();
			float dhx = elevationScale * 0.5f * float((h10 + h11) - (h00 + h01));
			float dhy = elevationScale * 0.5f * float((h01 + h11) - (h00 + h10));

			osg::Vec3f normal = osg::Vec3f(texelWorldSize.x() * filterWidthF, 0, dhx) ^ osg::Vec3f(0, texelWorldSize.y() * filterWidthF, dhy);
			normal.normalize();

			*p++ = std::clamp(int(normal.x() * 128.0f + 128.0f), 0, 255);
			*p++ = std::clamp(int(normal.y() * 128.0f + 128.0f), 0, 255);
			*p++ = std::clamp(int(normal.z() * 128.0f + 128.0f), 0, 255);
		}
	}
	return image;
}

} // namespace vis
} // namespace skybolt
