/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgImageHelpers.h"
#include "OsgMathHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Texture>
#include <osg/UserDataContainer>
#include <osgDB/ReadFile>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <assert.h>
#include <fstream>

namespace skybolt {
namespace vis {

osg::Vec4f applyGamma(const osg::Vec4f& c, float gamma)
{
	osg::Vec4f r = c;
	r.r() = pow(c.r(), gamma);
	r.g() = pow(c.g(), gamma);
	r.b() = pow(c.b(), gamma);
	return r;
}

osg::Vec4f averageSrgbColor(const osg::Image& image, float alphaRejectionThreshold)
{
	static const float gamma = srgbGamma();
	osg::Vec4f color;
	size_t pixels = 0;
	for (size_t t = 0; t < image.t(); ++t)
	{
		for (size_t s = 0; s < image.s(); ++s)
		{
			osg::Vec4f c = image.getColor(s, t);
			if (c.a() >= alphaRejectionThreshold)
			{
				color += srgbToLinear(c);
				++pixels;
			}
		}
	}

	color /= pixels;

	return linearToSrgb(color);
}

GLuint toSrgbInternalFormat(GLuint format)
{
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

	switch (format)
	{
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
	case GL_RGB:
	case GL_RGB8:
	case GL_SRGB8:
		return GL_SRGB8;
	case GL_RGBA:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
		return GL_SRGB8_ALPHA8;
	case GL_LUMINANCE:
	case GL_LUMINANCE8:
		return GL_SLUMINANCE8;
	case 0:
		return 0;
	default:
		assert(!"Not implemented");
	}
	return format;
}

osg::Image* readImageWithCorrectOrientation(const std::string& filename)
{
	using namespace boost::algorithm;
	osg::Image* image = osgDB::readImageFile(filename);
	if (!image)
		throw skybolt::Exception("Could not open image file: " + filename);

	std::string filenameLower = filename;
	to_lower(filenameLower);
	if (ends_with(filenameLower, "dds"))
		image->flipVertical();

	return image;
}

osg::ref_ptr<osg::Image> readImageWithoutWarnings(const std::string& filename)
{
	osgDB::ReaderWriter::ReadResult rr = osgDB::Registry::instance()->readImage(filename, osgDB::Registry::instance()->getOptions());
	if (rr.validImage()) return osg::ref_ptr<osg::Image>(rr.getImage());
	return nullptr;
}

osg::ref_ptr<osg::Image> readImageWithUserData(std::istream& s, const std::string& extension)
{
	osg::ref_ptr<osg::Image> image;
	{
		osg::ref_ptr<osgDB::ReaderWriter> reader = osgDB::Registry::instance()->getReaderWriterForExtension(extension);
		if (reader)
		{
			image = reader->readImage(s).takeImage();
		}
	}
	if (image && !s.eof())
	{
		auto reader = osgDB::Registry::instance()->getReaderWriterForExtension("osgb");
		osgDB::ReaderWriter::ReadResult result = reader->readObject(s);
		image->setUserDataContainer(result.takeObject()->asUserDataContainer());
	}
	return image;
}

bool writeImageWithUserData(const osg::Image& image, std::ostream& s, const std::string& extension)
{
	{
		osg::ref_ptr<osgDB::ReaderWriter> writer = osgDB::Registry::instance()->getReaderWriterForExtension(extension);
		if (!writer)
		{
			return false;
		}
		osgDB::ReaderWriter::WriteResult res = writer->writeImage(image, s);
		if (res.error())
		{
			return false;
		}
	}
	if (image.getUserDataContainer())
	{
		auto writer = osgDB::Registry::instance()->getReaderWriterForExtension("osgb");
		if (!writer)
		{
			return false;
		}
		osgDB::ReaderWriter::WriteResult res = writer->writeObject(*image.getUserDataContainer(), s);
		if (res.error())
		{
			return false;
		}
	}
	return true;
}

osg::Vec4f getColorBilinear(const osg::Image& image, const osg::Vec2f& coord)
{
	// Calculate coordinates
	int sMax = image.s() - 1;
	int tMax = image.t() - 1;

	osg::Vec2f clampedCoord;
	clampedCoord.x() = math::clamp(coord.x(), 0.0f, float(sMax));
	clampedCoord.y() = math::clamp(coord.y(), 0.0f, float(tMax));

	int u0 = (int)clampedCoord.x();
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)clampedCoord.y();
	int v1 = std::min(v0 + 1, tMax);

	// Calculate weights
	float fracU = clampedCoord.x() - u0;
	float fracV = clampedCoord.y() - v0;

	// Interpolate
	osg::Vec4f d00 = image.getColor(u0, v0);
	osg::Vec4f d10 = image.getColor(u1, v0);
	osg::Vec4f d01 = image.getColor(u0, v1);
	osg::Vec4f d11 = image.getColor(u1, v1);

	osg::Vec4f fracUVec(fracU, fracU, fracU, fracU);
	osg::Vec4f d0 = math::componentWiseLerp(d00, d10, fracUVec);
	osg::Vec4f d1 = math::componentWiseLerp(d01, d11, fracUVec);

	return math::componentWiseLerp(d0, d1, osg::Vec4f(fracV, fracV, fracV, fracV));
}

static osg::Vec4f componentWiseMin(const osg::Vec4f& a, const osg::Vec4f& b)
{
	osg::Vec4f r;
	for (int i = 0; i < osg::Vec4f::num_components; ++i)
	{
		r[i] = std::min(a[i], b[i]);
	}
	return r;
}

static osg::Vec4f componentWiseMax(const osg::Vec4f& a, const osg::Vec4f& b)
{
	osg::Vec4f r;
	for (int i = 0; i < osg::Vec4f::num_components; ++i)
	{
		r[i] = std::max(a[i], b[i]);
	}
	return r;
}

void normalize(osg::Image& image)
{
	constexpr float inf = std::numeric_limits<float>::infinity();
	osg::Vec4f cMin(inf, inf, inf, inf);
	osg::Vec4f cMax(0, 0, 0, 0);

	for (int z = 0; z < image.r(); ++z)
	{
		for (int y = 0; y < image.t(); ++y)
		{
			for (int x = 0; x < image.s(); ++x)
			{
				osg::Vec4f c = image.getColor(x, y, z);
				cMin = componentWiseMin(c, cMin);
				cMax = componentWiseMax(c, cMax);
			}
		}
	}

	for (int z = 0; z < image.r(); ++z)
	{
		for (int y = 0; y < image.t(); ++y)
		{
			for (int x = 0; x < image.s(); ++x)
			{
				osg::Vec4f c = image.getColor(x, y, z);
				c = osg::componentDivide((c - cMin), (cMax - cMin));
				image.setColor(c, x, y, z);
			}
		}
	}
}

bool isHeightMapDataFormat(const osg::Image& image)
{
	return image.getPixelFormat() == GL_LUMINANCE && image.getDataType() == GL_UNSIGNED_SHORT;
}

} // namespace vis
} // namespace skybolt
