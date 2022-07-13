/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "HeightMapElevationRerange.h"
#include <osg/Image>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Image> createNormalMapFromHeightMap(const osg::Image& heightmap, const HeightMapElevationRerange& rerange, const osg::Vec2f& texelWorldSize, int filterWidth = 1);

} // namespace vis
} // namespace skybolt
