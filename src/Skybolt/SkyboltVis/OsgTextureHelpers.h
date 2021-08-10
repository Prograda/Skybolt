/* Copyright 2012-2020 Matthew Reid
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

osg::ref_ptr<osg::Texture2D> createTilingSrgbTexture(const osg::ref_ptr<osg::Image>& image);

} // namespace vis
} // namespace skybolt
