/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "XyzTileSource.h"

#include "SkyboltVis/Elevation/ElevationBounds.h"
#include "SkyboltVis/Elevation/ElevationImageMetadata.h"
#include "SkyboltVis/Elevation/ElevationRerange.h"
#include "SkyboltVis/Image/ImageFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapHelpers.h"

#include <boost/algorithm/string/replace.hpp>
#include <SkyboltCommon/Logging/Logging.h>
#include <SkyboltCommon/ShaUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

XyzTileSource::XyzTileSource(const XyzTileSourceConfig& config) :
	TileSourceWithMinMaxLevel(config.levelRange),
	mImageFactory(config.imageFactory),
	mFileLocator(config.fileLocator),
	mUrlTemplate(config.urlTemplate),
	mYOrigin(config.yOrigin),
	mApiKey(config.apiKey),
	mCacheSha(skybolt::calcSha1(config.urlTemplate)),
	mElevationRerange(config.elevationRerange),
	mOptimizeElevationScale(config.optimizeElevationScale)
{
	assert(mImageFactory);
}

bool XyzTileSource::validate() const
{
	// Validate the loader by loading level 0 image
	std::string url = toUrl(QuadTreeTileKey());
	if (mFileLocator)
	{
		auto result = mFileLocator(url);
		if (!has_value(result))
		{
			SKYBOLT_LOG(error) << "Could not locate file for XyzTileSource with URL template '" << mUrlTemplate << "'";
			return false;
		}
		url = value(result)->string();
	}

	auto result = mImageFactory->readImage(url); // MTODO: support reading files over http in all relavent TileSource classes.
	if (!has_value(result))
	{
		SKYBOLT_LOG(error) << "Could not load image from XyzTileSource with URL template '" << mUrlTemplate << ". Reason: " << std::get<UnexpectedMessage>(result).str;
		return false;
	}

	if (mElevationRerange)
	{
		if (!isHeightMapDataFormat(**value(result)))
		{
			SKYBOLT_LOG(error) << "Elevation image with URL template '" << mUrlTemplate << "' is not a supported heightmap format.";
		}
		return false;
	}

	return true;
}

static int flipY(int y, int level)
{
	return (1 << level) - y - 1;
}

ImagePtr XyzTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	std::string url = toUrl(key);
	if (mFileLocator)
	{
		auto result = mFileLocator(url);
		if (!has_value(result))
		{
			return nullptr;
		}
		url = value(result)->string();
	}

	ImagePtr image = value(mImageFactory->readImage(url)).value_or(nullptr);
	if (image && isHeightMapDataFormat(*image))
	{
		image->setColorSpace(Image::ColorSpace::Linear);

		if (mElevationRerange)
		{
			ElevationRerange elevationRerange = *mElevationRerange;

			// Calculate elevation bounds
			ElevationBounds bounds = emptyElevationBounds();
			uint16_t* p = reinterpret_cast<uint16_t*>(image->getRawData());
			int elementCount = image->getWidth() * image->getHeight();
			for (int i = 0; i < elementCount; ++i)
			{
				expand(bounds, getElevationForColorValue(elevationRerange, p[i]));
			}

			// Optionally optimize scale to use full 0-65535 range.
			// This minimizes stepping artifacts when the heightmap is sampled with bilinear filtering.
			// Ideally this would be done when generating the heightmap tiles, rather than at load time.
			if (mOptimizeElevationScale)
			{
				int minValue = getColorValueForElevation(elevationRerange, bounds.x);
				int maxValue = getColorValueForElevation(elevationRerange, bounds.y);

				// Scale to full range
				float delta = float(maxValue - minValue);
				float scale = 65535.f / delta;
				for (int i = 0; i < elementCount; ++i)
				{
					p[i] = static_cast<uint16_t>(std::clamp(float(p[i] - minValue) * scale, 0.f, 65535.f));
				}

				// Update elevation rerange to match the new scale
				elevationRerange = rerangeElevationFromUInt16WithElevationBounds(bounds.x, bounds.y);
			}

			// Set image metadata
			ElevationImageMetadata metadata;
			metadata.elevationBounds = bounds;
			metadata.rerange = elevationRerange;
			setElevationImageMetadata(*image, metadata);
		}
	}

	return image;
}

std::string XyzTileSource::toUrl(const skybolt::QuadTreeTileKey& key) const
{
	int y = (mYOrigin == XyzTileSourceConfig::YOrigin::Top) ? key.y : flipY(key.y, key.level);

	std::string url = mUrlTemplate;
	boost::replace_all(url, "{x}", std::to_string(key.x));
	boost::replace_all(url, "{y}", std::to_string(y));
	boost::replace_all(url, "{z}", std::to_string(key.level));
	boost::replace_all(url, "{key}", mApiKey);
	return url;
}

} // namespace vis
} // namespace skybolt
