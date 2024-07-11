/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/OsgBox2.h"
#include <osg/Geometry>
#include <osg/Vec2>
#include <osg/Vec3>

namespace skybolt {
namespace vis {

osg::Geometry* createPlanetTileGeometry(const osg::Vec3d& tileCenter, const Box2d& latLonBounds,
	double radius, float skirtLength, PrimitiveType type);

osg::Geode* createPlanetTileGeode(const osg::Vec3d& tileCenter, const Box2d& latLonBounds, double radius, PrimitiveType type);

} // namespace vis
} // namespace skybolt
