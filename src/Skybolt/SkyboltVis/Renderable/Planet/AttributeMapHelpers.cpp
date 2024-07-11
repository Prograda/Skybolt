/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttributeMapHelpers.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"

#include <algorithm>

namespace skybolt {
namespace vis {

const AttributeColors& getNlcdAttributeColors()
{
	static AttributeColors c = {
		AttributeColor(0, osg::Vec4f(0.f, 0.f, 0.f, 1)), // none
		AttributeColor(0, osg::Vec4f(0.27843137255f, 0.41960784314f, 0.62745098039f, 1)), // water
		AttributeColor(0, osg::Vec4f(0.69803921569f, 0.67843137255f, 0.63921568628f, 1)), // barren
		// developed
		AttributeColor(0, osg::Vec4f(0.86666666667f, 0.78823529412f, 0.78823529412f, 1)), // developed, open space
		AttributeColor(0, osg::Vec4f(0.84705882353f, 0.57647058824f, 0.50980392157f, 1)), // developed, low
		AttributeColor(3, osg::Vec4f(0.92941176471f, 0.00000000000f, 0.00000000000f, 1)), // developed, med
		AttributeColor(4, osg::Vec4f(0.66666666667f, 0.00000000000f, 0.00000000000f, 1)), // developed, high
		// scrub
		AttributeColor(5, osg::Vec4f(0.64705882353f, 0.54901960784f, 0.18823529412f, 1)), // dwarf scrub
		AttributeColor(5, osg::Vec4f(0.80000000000f, 0.72941176471f, 0.48627450980f, 1)), // shrub scrub
		// grass
		AttributeColor(6, osg::Vec4f(0.88627450980f, 0.88627450980f, 0.75686274510f, 1)), // grassland
		AttributeColor(6, osg::Vec4f(0.78823529412f, 0.78823529412f, 0.46666666667f, 1)), // sedge
		AttributeColor(6, osg::Vec4f(0.60000000000f, 0.75686274510f, 0.27843137255f, 1)), // lichens
		AttributeColor(6, osg::Vec4f(0.46666666667f, 0.67843137255f, 0.57647058824f, 1)), // moss
		AttributeColor(6, osg::Vec4f(0.72941176471f, 0.84705882353f, 0.91764705882f, 1)), // woody wetlands
		AttributeColor(6, osg::Vec4f(0.43921568628f, 0.63921568628f, 0.72941176471f, 1)), // herbaceous wetlands
		// pasture
		AttributeColor(7, osg::Vec4f(0.85882352941f, 0.84705882353f, 0.23921568628f, 1)), // pasture
		// crops
		AttributeColor(8, osg::Vec4f(0.66666666667f, 0.43921568628f, 0.15686274510f, 1)), // cultivated crops
		// forest
		AttributeColor(9, osg::Vec4f(0.40784313726f, 0.66666666667f, 0.38823529412f, 1)), // deciduous forest
		AttributeColor(9, osg::Vec4f(0.10980392157f, 0.38823529412f, 0.18823529412f, 1)), // evergreen forest
		AttributeColor(9, osg::Vec4f(0.70980392157f, 0.78823529412f, 0.55686274510f, 1)), // mixed forest
		// snow
		AttributeColor(10, osg::Vec4f(0.81960784314f, 0.86666666667f, 0.97647058824f, 1)) // ice and snow
	};

	return c;
}


//! Writes a 3d texture of the RGB color space, i.e each axis has RGB values [0-255].
//! This texture is used by the artist as a basis to paint the mapping form RGB value to material ID.
static void writeRgbCubeTemplate(int width = 10)
{
	size_t size = width * width * width;

	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(width, width, width, GL_RGBA, GL_UNSIGNED_BYTE);

	unsigned char* p = image->data();
	for (size_t i = 0; i < size; ++i)
	{
		int x = i % width;
		int y = (i / width) % width;
		int z = i / (width * width);

		*p++ = float(x) / float(width) * 255;
		*p++ = float(y) / float(width) * 255;
		*p++ = float(z) / float(width) * 255;
		*p++ = 255;
	}

	writeTexture3d(*image, "TerrainMaterialColorCube.png");
}

//! @return material ID for a pixel of color c.
static int sampleMaterialColorMap(const osg::Vec4& c, const osg::Image& rgbToMaterialIdMappingCube)
{
	int x = std::clamp(int(c.x() * float(rgbToMaterialIdMappingCube.s())), 0, rgbToMaterialIdMappingCube.s() - 1);
	int y = std::clamp(int(c.y() * float(rgbToMaterialIdMappingCube.t())), 0, rgbToMaterialIdMappingCube.t() - 1);
	int z = std::clamp(int(c.z() * float(rgbToMaterialIdMappingCube.r())), 0, rgbToMaterialIdMappingCube.r() - 1);
	int v = std::round(255.0 * rgbToMaterialIdMappingCube.getColor(x, y, z).r());

	if (v == 0)
	{
		return 255;
	}
	else if (v == 255)
	{
		return 0;
	}
	else if (v == 48)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

osg::ref_ptr<osg::Image> convertToAttributeMap(const osg::Image& srcImage)
{
	static osg::ref_ptr<osg::Image> materialColorMap = readTexture3d("TerrainMaterialColorCube.png");

	osg::ref_ptr<osg::Image> dstImage(new osg::Image());
	dstImage->allocateImage(srcImage.s(),
		srcImage.t(),
		1,   // 2D texture is 1 pixel deep
		GL_RGBA,
		GL_UNSIGNED_BYTE);

	dstImage->setInternalTextureFormat(vis::toSrgbInternalFormat(GL_RGBA8));

	// Fill alpha channel with material ID
	unsigned char* p = (unsigned char*)dstImage->getDataPointer() + 3;
	for (size_t y = 0; y < srcImage.t(); ++y)
	{
		for (size_t x = 0; x < srcImage.s(); ++x)
		{
//#define DESPECKLE
#ifdef DESPECKLE
			std::map<int, int> idCounters;
			int kernalRadius = 2;
			for (int py = y - kernalRadius; py <= y + kernalRadius; ++py)
			{
				for (int px = x - kernalRadius; px <= x + kernalRadius; ++px)
				{
					if (px >= 0 && px < srcImage.s() &&
						py >= 0 && py < srcImage.t())
					{
						osg::Vec4 c = srcImage.getColor(px, py);
						int id = sampleMaterialColorMap(c, *materialColorMap);
						idCounters[id]++;
					}
				}
			}
			int bestId = 255;
			int bestIdCount = 0;
			for (const auto&[id, count] : idCounters)
			{
				if (count > bestIdCount)
				{
					bestId = id;
					bestIdCount = count;
				}
			}
			*p = bestId;
#else
			osg::Vec4 c = srcImage.getColor(x, y);
			int id = sampleMaterialColorMap(c, *materialColorMap);
			*p = id;
#endif
			p += 4;
		}
	}

	// Fill RGB channels with blurred copy of the albedo map
	p = (unsigned char*)dstImage->getDataPointer();
	for (int y = 0; y < srcImage.t(); ++y)
	{
		for (int x = 0; x < srcImage.s(); ++x)
		{
			int id = p[3];
			osg::Vec4 averagedColor(0, 0, 0, 0);
			int sampleCount = 0;

			int kernalRadius = 3;
			for (int py = y - kernalRadius; py <= y + kernalRadius; ++py)
			{
				for (int px = x - kernalRadius; px <= x + kernalRadius; ++px)
				{
					if (px >= 0 && px < srcImage.s() &&
						py >= 0 && py < srcImage.t())
					{
						int sampleId = std::round(255 * dstImage->getColor(px, py).a());
						bool canReplaceId = (id == 255 && sampleId != 255);
						if ((sampleId != 255 && sampleId == id) || canReplaceId)
						{
							if (canReplaceId)
							{
								id = sampleId;
								p[3] = id;
							}

							osg::Vec4f sourceColor = srcImage.getColor(px, py);
							if (sampleId == 1)
							{
								averagedColor += osg::Vec4f(0.02f, 0.1f, 0.02f, 0.0f)*0.9f;
							}
							else if (sampleId == 2)
							{
								averagedColor += osg::Vec4f(0.02f, 0.1f, 0.02f, 0.0f)*0.35f;
							}
							else
							{
								averagedColor += srgbToLinear(sourceColor);
							}
							++sampleCount;
						}
					}
				}
			}

			if (sampleCount > 0)
				averagedColor = linearToSrgb(averagedColor / sampleCount);
			else
				averagedColor = osg::Vec4(1.0, 1.0, 0.5, 1.0);
			*p++ = char(averagedColor.r() * 255);
			*p++ = char(averagedColor.g() * 255);
			*p = char(averagedColor.b() * 255);
			p += 2;
		}
	}

	return dstImage;
}

static int getAttributeWithNearestColor(const osg::Vec4f& c, const AttributeColors& colors)
{
	float bestDist = std::numeric_limits<float>::infinity();
	int bestColor = colors.begin()->first;
	for (AttributeColors::const_iterator i = colors.begin(); i != colors.end(); ++i)
	{
		osg::Vec4f diff = i->second - c;
		float dist = diff * diff;
		if (dist < bestDist)
		{
			bestDist = dist;
			bestColor = i->first;
		}
	}
	return bestColor;
}

osg::ref_ptr<osg::Image> convertAttributeMap(const osg::Image& srcImage, const AttributeColors& srcAttributeColors)
{
	if (srcAttributeColors.empty())
		throw skybolt::Exception("No attributes were found in source image");
	if (srcAttributeColors.size() > 256)
		throw skybolt::Exception("Number of attributes in source image must not exceed 256");

	osg::ref_ptr<osg::Image> dstImage(new osg::Image());
	dstImage->allocateImage(srcImage.s(),
							srcImage.t(),
							1,   // 2D texture is 1 pixel deep
							GL_LUMINANCE,
							GL_UNSIGNED_BYTE);

	dstImage->setInternalTextureFormat(GL_LUMINANCE8);

	char* p = (char*)dstImage->getDataPointer();

	for (size_t y = 0; y < srcImage.t(); ++y)
	{
		for (size_t x = 0; x < srcImage.s(); ++x)
		{
			osg::Vec4 c = srcImage.getColor(x, y);
#define FAST_ATTRIBUTE_CONVERSION_HACK
#ifdef FAST_ATTRIBUTE_CONVERSION_HACK
			// Fast path for trees only. Conversion is about 5x faster than using all attributes.
			int id = 0;
			{
				osg::Vec4f d = (c - osg::Vec4f(0.40784313726f, 0.66666666667f, 0.38823529412f, 1)); // deciduous forest
				if (d*d < 0.001)
					id = 9;
				else
				{
					osg::Vec4f d = (c - osg::Vec4f(0.10980392157f, 0.38823529412f, 0.18823529412f, 1)); // evergreen forest
					if (d*d < 0.001)
						id = 9;
					else
					{
						osg::Vec4f d = (c - osg::Vec4f(0.70980392157f, 0.78823529412f, 0.55686274510f, 1)); // mixed forest
						if (d*d < 0.001)
							id = 9;
					}
				}
			}
#else
			int id = getAttributeWithNearestColor(c, srcAttributeColors);
#endif
			*p++ = id;
		}
	}

	return dstImage;
}

} // namespace vis
} // namespace skybolt
