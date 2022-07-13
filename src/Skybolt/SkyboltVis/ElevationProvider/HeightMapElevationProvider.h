/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ElevationProvider.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"
#include <osg/Image>

namespace skybolt {
namespace vis {

class HeightMapElevationProvider : public ElevationProvider
{
public:
	HeightMapElevationProvider(const osg::ref_ptr<osg::Image>& image, const HeightMapElevationRerange& elevationRerange, const Box2f& bounds);

	//! @param x is latitude in radians
	//! @param y is longitude in radians
	float get(float x, float y) const;

private:
	osg::ref_ptr<const osg::Image> mImage;
	HeightMapElevationRerange mElevationRerange;
	const osg::Vec2f mHorizontalOffset;
	osg::Vec2f mHorizontalScale;
};

} // namespace vis
} // namespace skybolt
