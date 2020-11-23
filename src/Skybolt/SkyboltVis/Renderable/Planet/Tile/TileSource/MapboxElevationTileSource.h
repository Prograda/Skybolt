/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "XyzTileSource.h"

namespace skybolt {
namespace vis {

struct MapboxElevationTileSourceConfig
{
	//! URL must be templated with variables x, y, z in curley braces
	//! E.g "https://test.com/image/{z}/{x}/{y}.png"
	std::string urlTemplate;
};

class MapboxElevationTileSource : public TileSource
{
public:
	MapboxElevationTileSource(const MapboxElevationTileSourceConfig& config);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

private:
	std::unique_ptr<XyzTileSource> m_source;
};

} // namespace vis
} // namespace skybolt
