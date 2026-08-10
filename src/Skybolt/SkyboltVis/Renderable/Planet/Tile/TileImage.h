/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/QuadTree.h>
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {
namespace vis {

struct TileImage
{
	ImagePtr image; //!< Never null
	skybolt::QuadTreeTileKey key;
};

} // namespace vis
} // namespace skybolt
