/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OsgBox2.h"
#include <osg/Math>

namespace skybolt {
namespace vis {

inline Box2f getSubImageBounds(const Box2f& imageWorldBounds, const Box2f& subRegionWorldBounds, int imageWidth, int imageHeight)
{
	osg::Vec2f size = imageWorldBounds.maximum - imageWorldBounds.minimum;
	float widthF = float(imageWidth);
	float heightF = float(imageHeight);

	Box2f imageBounds;
	imageBounds.minimum.x() = osg::clampBetween(widthF * (subRegionWorldBounds.minimum.y() - imageWorldBounds.minimum.y()) / size.y(), 0.f, widthF);
	imageBounds.minimum.y() = osg::clampBetween(heightF * (subRegionWorldBounds.minimum.x() - imageWorldBounds.minimum.x()) / size.x(), 0.f, heightF);
	imageBounds.maximum.x() = osg::clampBetween(widthF * (subRegionWorldBounds.maximum.y() - imageWorldBounds.minimum.y()) / size.y(), 0.f, widthF);
	imageBounds.maximum.y() = osg::clampBetween(heightF * (subRegionWorldBounds.maximum.x() - imageWorldBounds.minimum.x()) / size.x(), 0.f, heightF);

	return imageBounds;
}

} // namespace vis
} // namespace skybolt
