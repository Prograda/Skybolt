/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileSource.h"

namespace skybolt {
namespace vis {

struct BingTileSourceConfig
{
	std::string url;
	std::string apiKey;
};

class BingTileSource : public TileSource
{
public:
	BingTileSource(const BingTileSourceConfig& config);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

private:
	std::string mUrlPartBeforeTileKey;
	std::string mUrlPartAfterTileKey;
};

} // namespace vis
} // namespace skybolt
