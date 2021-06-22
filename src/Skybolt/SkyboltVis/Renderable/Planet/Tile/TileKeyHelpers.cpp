/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileKeyHelpers.h"
#include <osg/Vec2i>

namespace skybolt {
namespace vis {

void getTileTransformInParentSpace(const QuadTreeTileKey& key, int parentLod, osg::Vec2f& scale, osg::Vec2f& offset)
{
	int reductions = key.level - parentLod;
	assert(reductions >= 0);

	int scaleInt = 1 << reductions;

	osg::Vec2i reducedIndex(key.x / scaleInt, key.y / scaleInt);
	osg::Vec2i v = reducedIndex * scaleInt;

	float rcpScale = 1.0f / (float)scaleInt;
	scale = osg::Vec2f(rcpScale, rcpScale);
	offset = osg::Vec2f(key.x - v.x(), (scaleInt - 1) - (key.y - v.y())) * rcpScale;
}

} // namespace vis
} // namespace skybolt
