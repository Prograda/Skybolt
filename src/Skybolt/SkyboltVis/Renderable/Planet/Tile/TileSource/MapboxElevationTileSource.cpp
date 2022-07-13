/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MapboxElevationTileSource.h"

#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"
#include <SkyboltCommon/ShaUtility.h>

#include <boost/algorithm/string/replace.hpp>
#include <osg/Texture>

using namespace skybolt;

namespace skybolt {
namespace vis {

MapboxElevationTileSource::MapboxElevationTileSource(const MapboxElevationTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mCacheSha(skybolt::calcSha1(config.urlTemplate + "__mapbox"))
{

	XyzTileSourceConfig xyzConfig;
	xyzConfig.urlTemplate = config.urlTemplate;
	xyzConfig.yOrigin = XyzTileSourceConfig::YOrigin::Top;
	xyzConfig.apiKey = config.apiKey;
	mSource = std::make_unique<XyzTileSource>(xyzConfig);
	mSource->validate();
}

osg::ref_ptr<osg::Image> MapboxElevationTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	osg::ref_ptr<osg::Image> image = mSource->createImage(key, cancelSupplier);

	if (image)
	{
		osg::ref_ptr<osg::Image> dest = new osg::Image();
		dest->allocateImage(image->s(), image->t(), 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
		dest->setInternalTextureFormat(getHeightMapInternalTextureFormat());

		const HeightMapElevationRerange& earthElevationRerange = getDefaultEarthRerange();
		setHeightMapElevationRerange(*dest, earthElevationRerange);

		const uint8_t* s = reinterpret_cast<uint8_t*>(image->data());
		uint16_t* d = reinterpret_cast<uint16_t*>(dest->data());

		HeightMapElevationBounds bounds = emptyHeightMapElevationBounds();

		size_t size = image->s() * image->t();
		for (size_t i = 0; i < size; ++i)
		{
			int r = *s++;
			int g = *s++;
			int b = *s++;
			s++; // a

			// Read mapbox elevation in meters. See https://docs.mapbox.com/data/tilesets/guides/access-elevation-data/
			float elevation = -10000.f + float(r * 256 * 256 + g * 256 + b) * 0.1f;

			// Store elevation as height map color value
			*d++ = getColorValueForElevation(earthElevationRerange, elevation);

			// Expand elevation bounds
			expand(bounds, elevation);
		}
		setHeightMapElevationBounds(*dest, bounds);

		return dest;
	}
	return nullptr;
}

} // namespace vis
} // namespace skybolt
