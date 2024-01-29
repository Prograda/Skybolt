/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Image>
#include <osgDB/Registry>
#include <SkyboltCommon/Exception.h>
#include <string>

namespace skybolt {
namespace vis {

inline float srgbGamma() { return 2.2f; }
inline float rcpSrgbGamma() { return 1.0f / srgbGamma(); }

osg::Vec4f applyGamma(const osg::Vec4f& c, float gamma);

inline osg::Vec4f srgbToLinear(const osg::Vec4f& c)
{
	return applyGamma(c, srgbGamma());
}

inline osg::Vec4f linearToSrgb(const osg::Vec4f& c)
{
	return applyGamma(c, rcpSrgbGamma());
}

inline void setPixelColor(osg::Image& image, int x, int y, const osg::Vec4f& color)
{
	// We convert from float color in range [0,1] to integer in range [0, 255] using rounding method because
	// it most accuratly represents original color.
	// See https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255/66862750#66862750
	if (image.getPixelFormat() == GL_RGBA && image.getDataType() == GL_UNSIGNED_BYTE)
	{
		unsigned char* p = &image.data()[4 * (x + image.s() * y)];
		*p++ = std::round(color.r() * 255.0f);
		*p++ = std::round(color.g() * 255.0f);
		*p++ = std::round(color.b() * 255.0f);
		*p = std::round(color.a() * 255.0f);
	}
	else if (image.getPixelFormat() == GL_RGB && image.getDataType() == GL_UNSIGNED_BYTE)
	{
		unsigned char* p = &image.data()[3 * (x + image.s() * y)];
		*p++ = std::round(color.r() * 255.0f);
		*p++ = std::round(color.g() * 255.0f);
		*p = std::round(color.b() * 255.0f);
	}
	else
	{
		throw skybolt::Exception("Unsupported image format");
	}
}

//! Returns the SRGB-space average colour of an SRGB-space image
osg::Vec4f averageSrgbColor(const osg::Image& image, float alphaRejectionThreshold = 0.5);

GLuint toSrgbInternalFormat(GLuint format);

osg::Image* readImageWithCorrectOrientation(const std::string& filename);

osg::ref_ptr<osg::Image> readImageWithoutWarnings(const std::string& filename, const osgDB::Options* options = osgDB::Registry::instance()->getOptions());

//! Reads an image from stream including the image's user data stored in osg::UserDataContainer
osg::ref_ptr<osg::Image> readImageWithUserData(std::istream& s, const std::string& extension);
//! Write an image to a stream including the image's user data stored in osg::UserDataContainer.
//! @return true on success.
bool writeImageWithUserData(const osg::Image& image, std::ostream& s, const std::string& extension);

//! @param coord is in pixel coordinates (not normalized)
osg::Vec4f getColorBilinear(const osg::Image& image, const osg::Vec2f& coord);

void normalize(osg::Image& image);

bool isHeightMapDataFormat(const osg::Image& image);

} // namespace vis
} // namespace skybolt
