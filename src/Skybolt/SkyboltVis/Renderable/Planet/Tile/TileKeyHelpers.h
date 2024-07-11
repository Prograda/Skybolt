/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Vec2>

using namespace skybolt;

namespace skybolt {
namespace vis {

void getTileTransformInParentSpace(const QuadTreeTileKey& key, int parentLod, osg::Vec2f& scale, osg::Vec2f& offset);

} // namespace vis
} // namespace skybolt
