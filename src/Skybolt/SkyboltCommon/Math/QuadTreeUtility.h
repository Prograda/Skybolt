/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "QuadTree.h"

namespace skybolt {

//! Visits all tiles in the hierarchy, starting from the root, up to and including the targetKey
//! @returns the leaf-most tile visited
template <typename TileT>
TileT& visitHierarchyToKey(TileT& tile, const skybolt::QuadTreeTileKey& targetKey, std::function<void(TileT&)> visitor)
{
	assert(createAncestorKey(targetKey, tile.key.level) == tile.key);
	visitor(tile);

	if (tile.key == targetKey)
	{
		return tile;
	}

	if (tile.hasChildren())
	{
		const QuadTreeTileKey& childKey = createAncestorKey(targetKey, tile.key.level + 1);
		for (int i = 0; i < 4; ++i)
		{
			TileT& child = static_cast<TileT&>(*tile.children[i]);
			if (child.key == childKey)
			{
				return visitHierarchyToKey(child, targetKey, visitor);
			}
		}
	}
	return tile;
}

//! Visits all tiles in a hierarchy 
//! @param shouldTraverse returns true if the children of this tile should be visited
template <typename TileT>
void visitHierarchy(TileT& tile, std::function<bool(const TileT&)> shouldTraverse, std::function<void(TileT&)> visitor)
{
	if (!shouldTraverse(tile))
		return;

	visitor(tile);

	if (tile.hasChildren())
	{
		for (int i = 0; i < 4; ++i)
		{
			visitHierarchy(static_cast<TileT&>(*tile.children[i]), shouldTraverse, visitor);
		}
	}
}

} // namespace skybolt
