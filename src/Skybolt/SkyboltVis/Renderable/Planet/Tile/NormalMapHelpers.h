/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Elevation/ElevationRerange.h"

namespace skybolt {
namespace vis {

ImagePtr createNormalMapFromHeightMap(const ImageFactory& imageFactory, const Image& heightmap, const ElevationRerange& rerange, const glm::vec2& texelWorldSize, int filterWidth = 1);

} // namespace vis
} // namespace skybolt
