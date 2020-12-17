/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OsgMathHelpers.h"
#include <SkyboltCommon/Math/Box2.h>
#include <osg/BoundingBox>
#include <osg/Vec2f>
#include <osg/Vec2d>
#include <osg/Vec2i>

namespace skybolt {
namespace vis {

typedef skybolt::Box2T<osg::Vec2f> Box2f;
typedef skybolt::Box2T<osg::Vec2d> Box2d;
typedef skybolt::Box2T<osg::Vec2i> Box2i;

typedef osg::BoundingBoxImpl<osg::Vec2f> BoundingBox2f; // TODO: no need to use our own type, and an osg type

} // namespace vis
} // namespace skybolt
