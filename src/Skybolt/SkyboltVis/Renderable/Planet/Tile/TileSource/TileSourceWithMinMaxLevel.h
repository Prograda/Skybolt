/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TileSource.h"
#include <SkyboltCommon/Range.h>

namespace skybolt {
namespace vis {

class TileSourceWithMinMaxLevel : public TileSource
{
public:
	TileSourceWithMinMaxLevel(const IntRangeInclusive& levelRange);
	~TileSourceWithMinMaxLevel() override = default;

	bool hasAnyChildren(const skybolt::QuadTreeTileKey& key) const override;

	std::optional<skybolt::QuadTreeTileKey> getHighestAvailableLevel(const skybolt::QuadTreeTileKey& key) const override;

private:
	IntRangeInclusive mLevelRange;
};

} // namespace vis
} // namespace skybolt
