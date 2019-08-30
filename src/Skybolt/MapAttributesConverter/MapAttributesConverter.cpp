/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MapAttributesConverter.h"
#include <SkyboltCommon/Exception.h>
#include <limits>

namespace skybolt {

int getAttributeWithNearestColor(const osg::Vec4f& c, const AttributeColors& colors)
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

} // namespace skybolt