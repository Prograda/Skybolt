/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SphericalMercatorToPlateCarreeTileSource.h"
#include "SkyboltVis/Elevation/ElevationBounds.h"
#include "SkyboltVis/Elevation/ElevationImageMetadata.h"
#include "SkyboltVis/Elevation/ElevationRerange.h"
#include "SkyboltVis/Image/ImageFactory.h"
#include "SkyboltVis/Image/ImageUtil.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapHelpers.h"

#include <SkyboltCommon/Math/MathUtility.h>

#include <httplib.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

static glm::ivec2 toVec2i(const glm::vec2& v)
{
	return glm::ivec2(v.x, v.y);
}

// From: https://docs.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system
//! @Return map width and height in pixels
static int calcMapSize(int levelOfDetail)
{
	return 256 << levelOfDetail;
}

//! Converts a point from latitude/longitude WGS-84 coordinates (in degrees)  
//! into pixel XY in spherical mercator coordinates at a specified level of detail.  
static glm::vec2 latLongToPixelXY(double latitude, double longitude, int levelOfDetail)
{
	static const double MinLatitude = -85.05112878 * math::degToRadD();
	static const double MaxLatitude = 85.05112878 * math::degToRadD();
	latitude = math::clamp(latitude, MinLatitude, MaxLatitude);

	double x = (longitude + math::piD()) / math::twoPiD();
	double sinLatitude = std::sin(latitude);
	double y = 0.5 - std::log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * math::piD());

	float mapSize = (float)calcMapSize(levelOfDetail);

	return glm::vec2(
		math::clamp(float(x * mapSize), 0.f, mapSize),
		math::clamp(float(y * mapSize), 0.f, mapSize));
}

static glm::ivec2 pixelXYToTileXY(const glm::ivec2& pixelXy)
{
	return pixelXy / 256;
}

static Box2i getSphericalMercatorKeysCoveringBoundsAtLevel(int level, const Box2d& keyBounds)
{
	glm::vec2 minBound = latLongToPixelXY(keyBounds.maximum.x, keyBounds.minimum.y, level) + glm::vec2(0.5f, 0.5f);
	glm::vec2 maxBound = latLongToPixelXY(keyBounds.minimum.x, keyBounds.maximum.y, level) - glm::vec2(0.5f, 0.5f);
	return Box2i(
		pixelXYToTileXY(toVec2i(minBound)),
		pixelXYToTileXY(toVec2i(maxBound))
	);
}


SphericalMercatorToPlateCarreeTileSource::SphericalMercatorToPlateCarreeTileSource(const ImageFactoryPtr& imageFactory, const TileSourcePtr& source) :
	mImageFactory(imageFactory),
	mTileSource(source)
{
	assert(mImageFactory);
	assert(mTileSource);
}

struct ivec2Compare
{
   bool operator() (const glm::ivec2& a, const glm::ivec2& b) const
   {
      return std::tie(a.x, a.y) < std::tie(b.x, b.y);
   }
};

ImagePtr SphericalMercatorToPlateCarreeTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	// Find the bounds of the PlateCarree tile in SpericalMercator space
	Box2d keyBounds = getKeyLatLonBounds<glm::dvec2>(key);
	int sphericalMercatorLevel = key.level + 1; // Spherical mercator tiles half half the resolution, so we need to look at one level higher.
	Box2i tilesBounds = getSphericalMercatorKeysCoveringBoundsAtLevel(sphericalMercatorLevel, keyBounds);

	// The tile bounds give us all the Sperical Mercator tiles that the Plate Carree tile intersects.
	// Download each tile.
	Image::Format format;
	Image::ColorSpace colorSpace;
	std::map<glm::ivec2, ImagePtr, ivec2Compare> tiles;
	std::optional<ElevationImageMetadata> metadata;
	for (int y = tilesBounds.minimum.y; y <= tilesBounds.maximum.y; ++y)
	{
		for (int x = tilesBounds.minimum.x; x <= tilesBounds.maximum.x; ++x)
		{
			// FIXME: Consider storing tiles in an LRU so that we don't need to re-download the same tile.
			// It might not help very much because in practice only about 10% of tiles are re-downloaded.
			// The cache must be thread-safe because a TileSource can be queried from multiple threads.
			ImagePtr image = mTileSource->createImage(QuadTreeTileKey(sphericalMercatorLevel, x, y), cancelSupplier);
			if (image)
			{
				tiles[glm::ivec2(x, y)] = image;
				format = image->getFormat();
				colorSpace = image->getColorSpace();

				auto sourceMetadata = getElevationImageMetadata(*image);
				if (sourceMetadata)
				{
					if (!metadata)
					{
						metadata = ElevationImageMetadata();
						metadata->rerange = sourceMetadata->rerange;
					}
					else if (sourceMetadata->rerange != metadata->rerange)
					{
						throw std::runtime_error("Source tiles have inconsistant elevation ranges");
					}

					expand(metadata->elevationBounds, sourceMetadata->elevationBounds);
				}
			}
			else
			{
				// Image not available
				return nullptr;
			}

			if (cancelSupplier())
			{
				return nullptr;
			}
		}
	}

	if (tiles.empty())
	{
		return nullptr;
	}

	// Composite the Spherical Mercator tiles into a single Plate Carree tile and return it.
	ImagePtr composite = valueOrThrowException(mImageFactory->createImage(256, 256, format, colorSpace));

	if (isHeightMapDataFormat(*composite))
	{
		composite->setColorSpace(Image::ColorSpace::Linear);
	}

	glm::dvec2 size = keyBounds.size();
	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			glm::dvec2 latLon = keyBounds.minimum + glm::dvec2(size.x * (double(y) + 0.5) / 256.0, size.y * (double(x) + 0.5) / 256.0);
			glm::vec2 srcXy = latLongToPixelXY(latLon.x, latLon.y, sphericalMercatorLevel);

			auto it = tiles.find(pixelXYToTileXY(toVec2i(srcXy)));
			if (it != tiles.end())
			{
				const Image& src = *it->second;
				glm::vec4 color = getColorBilinear(src, glm::vec2(fmodf(srcXy.x, 256.f), 255.f - fmodf(srcXy.y, 256.f)));
				composite->setColor(x, y, color);
			}
		}
	}

	if (metadata)
	{
		setElevationImageMetadata(*composite, *metadata);
	}

	return composite;
}

} // namespace vis
} // namespace skybolt
