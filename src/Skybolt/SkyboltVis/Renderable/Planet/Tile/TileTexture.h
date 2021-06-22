/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct TileTexture
{
	osg::ref_ptr<osg::Texture2D> texture;
	skybolt::QuadTreeTileKey key;
};

} // namespace vis
} // namespace skybolt
