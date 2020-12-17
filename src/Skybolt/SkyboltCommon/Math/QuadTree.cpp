/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QuadTree.h"

namespace skybolt {

QuadTreeTileKey createAncestorKey(const QuadTreeTileKey& key, int level)
{
	int reductions = key.level - level;
	assert(reductions >= 0);

	int scaleInt = 1 << reductions;

	QuadTreeTileKey ancestor;
	ancestor.level = level;
	ancestor.x = key.x / scaleInt;
	ancestor.y = key.y / scaleInt;
	return ancestor;
}

} // namespace skybolt
