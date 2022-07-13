/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "TileSourceWithMinMaxLevel.h"

namespace skybolt {
namespace vis {

struct BingTileSourceConfig
{
	std::string url;
	std::string apiKey;
	IntRangeInclusive levelRange;
};

class BingTileSource : public TileSourceWithMinMaxLevel
{
public:
	BingTileSource(const BingTileSourceConfig& config);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;
	const std::string& getCacheSha() const override { return mCacheSha; }

private:
	const std::string mCacheSha;
	std::string mUrlPartBeforeTileKey;
	std::string mUrlPartAfterTileKey;
};

} // namespace vis
} // namespace skybolt
