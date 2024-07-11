/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Image>

namespace skybolt {
namespace vis {

class TerrainForestTextureGenerator
{
public:
	static osg::ref_ptr<osg::Image> generate(const osg::Image& billboardTexture);
};

} // namespace vis
} // namespace skybolt
