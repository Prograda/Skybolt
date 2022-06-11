/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MapboxElevationTileSource.h"

#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMap.h"
#include <boost/algorithm/string/replace.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

MapboxElevationTileSource::MapboxElevationTileSource(const MapboxElevationTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange)
{
	XyzTileSourceConfig xyzConfig;
	xyzConfig.urlTemplate = config.urlTemplate;
	xyzConfig.yOrigin = XyzTileSourceConfig::YOrigin::Top;
	xyzConfig.apiKey = config.apiKey;
	m_source = std::make_unique<XyzTileSource>(xyzConfig);
	m_source->validate();
}

osg::ref_ptr<osg::Image> MapboxElevationTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	osg::ref_ptr<osg::Image> image = m_source->createImage(key, cancelSupplier);

	if (image)
	{
		osg::ref_ptr<osg::Image> dest = new osg::Image();
		dest->allocateImage(image->s(), image->t(), 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);

		const uint8_t* s = reinterpret_cast<uint8_t*>(image->data());
		uint16_t* d = reinterpret_cast<uint16_t*>(dest->data());

		size_t size = image->s() * image->t();
		for (size_t i = 0; i < size; ++i)
		{
			int r = *s++;
			int g = *s++;
			int b = *s++;
			s++; // a

			int v = -10000 + int(float(r * 256 * 256 + g * 256 + b) * 0.1f);
			if (v <= 0)
			{
			//	v = -500;
			}

			*d++ = v + getHeightmapSeaLevelValueInt();
		}

		return dest;
	}
	return nullptr;
}

} // namespace vis
} // namespace skybolt
