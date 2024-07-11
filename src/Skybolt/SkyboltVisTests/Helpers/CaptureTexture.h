/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/Texture>

// @param imageType is pixel data type for the image to produce e.g GL_FLOAT, GL_UNSIGNED_BYTE etc
osg::ref_ptr<osg::Image> captureTexture(osg::RenderInfo& renderInfo, const osg::Texture& texture, GLenum imageType);

class ImageCaptureDrawCallback : public osg::Camera::DrawCallback
{
public:

	struct CaptureItem
	{
		CaptureItem(const osg::ref_ptr<osg::Texture>& texture, GLenum imageFormat) :
			texture(texture), imageFormat(imageFormat) {}

		osg::ref_ptr<osg::Texture> texture;
		GLenum imageFormat;
	};

	ImageCaptureDrawCallback(const std::vector<CaptureItem>& texturesToCapture) :
		mTexturesToCapture(texturesToCapture)
	{
	}

	void operator () (osg::RenderInfo& renderInfo) const override
	{
		for (const auto& item : mTexturesToCapture)
		{
			capturedImages.push_back(captureTexture(renderInfo, *item.texture, item.imageFormat));
		}
	}

	void setTexturesToCapture(const std::vector<CaptureItem>& texturesToCapture)
	{
		mTexturesToCapture = texturesToCapture;
	}

	mutable std::vector<osg::ref_ptr<osg::Image>> capturedImages;

private:
	std::vector<CaptureItem> mTexturesToCapture;
};