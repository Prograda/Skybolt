/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MapboxElevationTileSource.h"

#include "SkyboltVis/Elevation/ElevationBounds.h"
#include "SkyboltVis/Elevation/ElevationImageMetadata.h"
#include "SkyboltVis/Elevation/ElevationRerange.h"
#include "SkyboltVis/Image/ImageFactory.h"

#include <SkyboltCommon/ShaUtility.h>

#include <boost/algorithm/string/replace.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

MapboxElevationTileSource::MapboxElevationTileSource(const MapboxElevationTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mImageFactory(config.imageFactory),
	mCacheSha(skybolt::calcSha1(config.urlTemplate + "__mapbox"))
{
	assert(mImageFactory);

	XyzTileSourceConfig xyzConfig;
	xyzConfig.urlTemplate = config.urlTemplate;
	xyzConfig.yOrigin = XyzTileSourceConfig::YOrigin::Top;
	xyzConfig.apiKey = config.apiKey;
	xyzConfig.imageFactory = mImageFactory;
	mSource = std::make_unique<XyzTileSource>(xyzConfig);
	mSource->validate();
}

ImagePtr MapboxElevationTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	ImagePtr image = mSource->createImage(key, cancelSupplier);

	if (image)
	{
		ImagePtr dest = valueOrThrowException(mImageFactory->createImage(image->getWidth(), image->getHeight(), Image::Format::R16, Image::ColorSpace::Linear));

		// Fill image with converted elevation data
		const uint8_t* s = reinterpret_cast<uint8_t*>(image->getRawData());
		uint16_t* d = reinterpret_cast<uint16_t*>(dest->getRawData());

		ElevationBounds bounds = emptyElevationBounds();
		const ElevationRerange& earthElevationRerange = getDefaultEarthRerange();

		size_t size = image->getWidth() * image->getHeight();
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

		ElevationImageMetadata metadata;
		metadata.elevationBounds = bounds;
		metadata.rerange = earthElevationRerange;
		setElevationImageMetadata(*image, metadata);

		return dest;
	}
	return nullptr;
}

} // namespace vis
} // namespace skybolt
