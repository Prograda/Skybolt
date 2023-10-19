/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgTextureHelpers.h"
#include "OsgImageHelpers.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <iostream>
#include <iomanip>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Texture2D> createRenderTexture(int width, int height)
{
#ifdef USE_DELL_XPS_RTT_FIX
	// Fixes bug on integrated Dell xps 13 graphics where textures attached to frame buffer render black if image is not allocated first
	osg::Image* outputImage = new osg::Image();
	outputImage->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	osg::Texture2D* texture = new osg::Texture2D(outputImage);
#else
	osg::Texture2D* texture = new osg::Texture2D();
#endif
	texture->setTextureWidth(width);
	texture->setTextureHeight(height);
	return texture;
}

void writeTexture3d(const osg::Image& image, const std::string& filename)
{
	osg::ref_ptr<osg::Image> dstImage = new osg::Image;
	dstImage->allocateImage(image.s(), image.t() * image.r(), 1, image.getPixelFormat(), image.getDataType());
	memcpy(dstImage->data(), image.data(), image.getTotalDataSize());

	osgDB::writeImageFile(*dstImage, filename);
}

osg::ref_ptr<osg::Image> readTexture3d(const std::string& filename)
{
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);

	int height = image->t() / image->s();

	osg::ref_ptr<osg::Image> dstImage = new osg::Image;
	dstImage->allocateImage(image->s(), height, height, image->getPixelFormat(), image->getDataType());
	memcpy(dstImage->data(), image->data(), image->getTotalDataSize());

	return dstImage;
}

osg::ref_ptr<osg::Image> readTexture3dFromSeparateFiles(const std::string& filenamePrefix, const std::string& extension, int depth)
{
	osg::ref_ptr<osg::Image> dstImage;

	for (int i = 0; i < depth; ++i)
	{
		std::stringstream ss;
		ss << filenamePrefix << std::setfill('0') << std::setw(3) << (i + 1) << extension;
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile(ss.str());
		if (!image)
		{
			throw std::runtime_error("Could not open file: " + ss.str());
		}

		if (!dstImage)
		{
			dstImage = new osg::Image;
			dstImage->allocateImage(image->s(), image->t(), depth, image->getPixelFormat(), image->getDataType());
		}

		memcpy(dstImage->data() + i * image->getTotalDataSize(), image->data(), image->getTotalDataSize());
	}

	return dstImage;
}

osg::ref_ptr<osg::Texture2D> createSrgbTexture(const osg::ref_ptr<osg::Image>& image)
{
	if (image) // since it is valid to create osg::Texture2D with a null image, we handle this case here
	{
		// Set the SRGB format for the image before creating the texture, so that the image format matches the texture.
		// DDS textures do not display correctly if the image format does not match the texture format.
		image->setInternalTextureFormat(toSrgbInternalFormat(image->getInternalTextureFormat()));
	}

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	return texture;
}

osg::ref_ptr<osg::Texture2D> createTilingSrgbTexture(const osg::ref_ptr<osg::Image>& image)
{
	osg::ref_ptr<osg::Texture2D> texture = createSrgbTexture(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	return texture;
}

ScaleOffset calcHalfTexelEdgeRemovalScaleOffset(const osg::Vec2i& textureSize)
{
	ScaleOffset r;
	r.scale = osg::Vec2f(float(textureSize.x() - 1) / textureSize.x(), float(textureSize.y() - 1) / textureSize.y());
	r.offset = osg::Vec2f(0.5f / textureSize.x(), 0.5f / textureSize.y());
	return r;
}

} // namespace vis
} // namespace skybolt
