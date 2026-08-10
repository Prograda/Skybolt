/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/QuadTree.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

void getTileTransformInParentSpace(const QuadTreeTileKey& key, int parentLod, glm::vec2& scale, glm::vec2& offset, bool flipV = false);

} // namespace vis
} // namespace skybolt
