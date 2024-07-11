/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Texture2D>
#include <string>

namespace skybolt {
namespace vis {

//! Creates a texture which can act as a render target
osg::ref_ptr<osg::Texture2D> createRenderTexture(int width, int height);

void writeTexture3d(const osg::Image& image, const std::string& filename);

osg::ref_ptr<osg::Image> readTexture3d(const std::string& filename);

osg::ref_ptr<osg::Image> readTexture3dFromSeparateFiles(const std::string& filenamePrefix, const std::string& extension, int depth);

osg::ref_ptr<osg::Texture2D> createSrgbTexture(const osg::ref_ptr<osg::Image>& image);
osg::ref_ptr<osg::Texture2D> createTilingSrgbTexture(const osg::ref_ptr<osg::Image>& image);

inline int getHeightMapInternalTextureFormat()
{
	static const int f = GL_R16;
	return f;
}

struct ScaleOffset
{
	osg::Vec2f scale;
	osg::Vec2f offset;
};

//! Because OpenGL texels are in center of texture pixels, when GL_CLAMP_EDGE is used
//! there is a margin of half a texel around the edges of the texture.
//! For some use cases, such as height maps, we need to remove this margin so that
//! The resulting UVs have [0, 1] range from the first pixel center to the last pixel center.
//! This function calculates the UV scale and offset required to remove the margin., such that
//! newUv = oldUv * scale + offset.
ScaleOffset calcHalfTexelEdgeRemovalScaleOffset(const osg::Vec2i& textureSize);

} // namespace vis
} // namespace skybolt
