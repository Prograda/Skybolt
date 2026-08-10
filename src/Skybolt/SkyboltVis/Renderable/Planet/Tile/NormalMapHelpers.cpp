/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NormalMapHelpers.h"
#include "SkyboltVis/Image/ImageFactory.h"

#include <algorithm>
#include <assert.h>

namespace skybolt {
namespace vis {

ImagePtr createNormalMapFromHeightMap(const ImageFactory& imageFactory, const Image& heightmap, const ElevationRerange& rerange, const glm::vec2& texelWorldSize, int filterWidth)
{
	assert(heightmap.getFormat() == Image::Format::R16);
	const int width = heightmap.getWidth();
	const int height = heightmap.getHeight();

	ImagePtr image = valueOrThrowException(imageFactory.createImage(width, height, Image::Format::RGB8, Image::ColorSpace::Linear));

	unsigned char* p = image->getRawData();
	const uint16_t* src = reinterpret_cast<const uint16_t*>(heightmap.getRawData());

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

			const float elevationScale = rerange.x;
			float dhx = elevationScale * 0.5f * float((h10 + h11) - (h00 + h01));
			float dhy = elevationScale * 0.5f * float((h01 + h11) - (h00 + h10));

			glm::dvec3 normal = glm::cross(
				glm::dvec3(texelWorldSize.x * filterWidthF, 0, dhx),
				glm::dvec3(0, texelWorldSize.y * filterWidthF, dhy)
				);
			normal = glm::normalize(normal);

			*p++ = std::clamp(int(normal.x * 128.0f + 128.0f), 0, 255);
			*p++ = std::clamp(int(normal.y * 128.0f + 128.0f), 0, 255);
			*p++ = std::clamp(int(normal.z * 128.0f + 128.0f), 0, 255);
		}
	}
	return image;
}

} // namespace vis
} // namespace skybolt
