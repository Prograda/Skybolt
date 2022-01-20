/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ElevationProvider.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMap.h"
#include <osg/Image>

namespace skybolt {
namespace vis {

//@param take value as float instead of uint16_t so we can handle conversion of non integer heights produced by interpolation.
inline float heightmapValueToFloat(float value)
{
	return -value + getHeightmapSeaLevelValueFloat();
}

inline uint16_t floatToHeightmapValue(float z)
{
	return uint16_t(-z + getHeightmapSeaLevelValueFloat());
}

class HeightmapElevationProvider : public ElevationProvider
{
public:
	HeightmapElevationProvider(const osg::ref_ptr<osg::Image>& image, const Box2f& bounds);

	//! @param x is latitude in radians
	//! @param y is longitude in radians
	float get(float x, float y) const;

private:
	osg::ref_ptr<const osg::Image> image;
	const osg::Vec2f offset;
	osg::Vec2f scale;
};

} // namespace vis
} // namespace skybolt
