/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CaptureTexture.h"
#include <osg/Texture3D>

osg::ref_ptr<osg::Image> captureTexture(osg::RenderInfo& renderInfo, const osg::Texture& texture, GLenum imageType)
{
	// FIXME: This is workaround for a limitation of osg::Image::readImageFromCurrentTexture
	// where, when both a 2D and 3D texture are bound, the 2D texture will always be captured
	// instead of the 3D texture. If we want to capture a 3D texture, we need to manually
	// unbind the 2D texture first. This requires a raw openGL call.
	if (dynamic_cast<const osg::Texture3D*>(&texture))
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	renderInfo.getState()->applyAttribute(&texture);
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->readImageFromCurrentTexture(renderInfo.getContextID(), false, imageType);

	return image;
}
