/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileSourceWithMinMaxLevel.h"

namespace skybolt {
namespace vis {

TileSourceWithMinMaxLevel::TileSourceWithMinMaxLevel(const IntRangeInclusive& levelRange) :
	mLevelRange(levelRange)
{
}

bool TileSourceWithMinMaxLevel::hasAnyChildren(const skybolt::QuadTreeTileKey& key) const
{
	return key.level < mLevelRange.maximum;
}

std::optional<skybolt::QuadTreeTileKey> TileSourceWithMinMaxLevel::getHighestAvailableLevel(const skybolt::QuadTreeTileKey& requestedKey) const
{
	if (requestedKey.level >= mLevelRange.minimum)
	{
		return createAncestorKey(requestedKey, std::min(requestedKey.level, mLevelRange.maximum));
	}
	return std::nullopt;
}

} // namespace vis
} // namespace skybolt
