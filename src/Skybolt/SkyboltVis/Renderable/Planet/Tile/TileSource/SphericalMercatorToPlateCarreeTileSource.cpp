/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SphericalMercatorToPlateCarreeTileSource.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <httplib/httplib.h>
#include <osg/Vec2i>

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>

using namespace skybolt;

namespace skybolt {
namespace vis {

static osg::Vec2i toVec2i(const osg::Vec2f& v)
{
	return osg::Vec2i(v.x(), v.y());
}

// From: https://docs.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system
//! @Return map width and height in pixels
static int calcMapSize(int levelOfDetail)
{
	return 256 << levelOfDetail;
}

//! Converts a point from latitude/longitude WGS-84 coordinates (in degrees)  
//! into pixel XY coordinates at a specified level of detail.  
static osg::Vec2f latLongToPixelXY(double latitude, double longitude, int levelOfDetail)
{
	static const double MinLatitude = -85.05112878 * math::degToRadD();
	static const double MaxLatitude = 85.05112878 * math::degToRadD();
	latitude = math::clamp(latitude, MinLatitude, MaxLatitude);

	double x = (longitude + math::piD()) / math::twoPiD();
	double sinLatitude = std::sin(latitude);
	double y = 0.5 - std::log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * math::piD());

	float mapSize = (float)calcMapSize(levelOfDetail);

	return osg::Vec2f(
		math::clamp(float(x * mapSize), 0.f, mapSize),
		math::clamp(float(y * mapSize), 0.f, mapSize));
}

static osg::Vec2i pixelXYToTileXY(const osg::Vec2i& pixelXy)
{
	return pixelXy / 256;
}

static Box2i convertPlateCarreeToSphericalMercator(const QuadTreeTileKey& key, const Box2d& keyBounds)
{
	osg::Vec2f minBound = latLongToPixelXY(keyBounds.maximum.x(), keyBounds.minimum.y(), key.level) + osg::Vec2f(0.5f, 0.5f);
	osg::Vec2f maxBound = latLongToPixelXY(keyBounds.minimum.x(), keyBounds.maximum.y(), key.level) - osg::Vec2f(0.5f, 0.5f);
	return Box2i(
		pixelXYToTileXY(toVec2i(minBound)),
		pixelXYToTileXY(toVec2i(maxBound))
	);
}


SphericalMercatorToPlateCarreeTileSource::SphericalMercatorToPlateCarreeTileSource(const TileSourcePtr& source) :
	mTileSource(source)
{
	assert(mTileSource);
}

osg::ref_ptr<osg::Image> SphericalMercatorToPlateCarreeTileSource::createImage(const QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
{
	// Find the bounds of the PlateCarree tile in SpericalMercator space
	Box2d keyBounds = getKeyLatLonBounds<osg::Vec2d>(key);
	QuadTreeTileKey quadTreeTileKey = key;
	quadTreeTileKey.level += 1;
	Box2i tilesBounds = convertPlateCarreeToSphericalMercator(quadTreeTileKey, keyBounds);

	// The tile bounds give us all the Sperical Mercator tiles that the Plate Carree tile intersects.
	// Download each tile.
	GLenum pixelFormat;
	GLenum type;
	std::map<osg::Vec2i, osg::ref_ptr<osg::Image>> tiles;
	std::optional<HeightMapElevationBounds> bounds;
	std::optional<HeightMapElevationRerange> rerange;
	for (int y = tilesBounds.minimum.y(); y <= tilesBounds.maximum.y(); ++y)
	{
		for (int x = tilesBounds.minimum.x(); x <= tilesBounds.maximum.x(); ++x)
		{
			// FIXME: Consider storing tiles in an LRU so that we don't need to re-download the same tile.
			// It might not help very much because in practice only about 10% of tiles are re-downloaded.
			// The cache must be thread-safe because a TileSource can be queried from multiple threads.
			osg::ref_ptr<osg::Image> image = mTileSource->createImage(QuadTreeTileKey(quadTreeTileKey.level, x, y), cancelSupplier);
			if (image)
			{
				tiles[osg::Vec2i(x, y)] = image;
				pixelFormat = image->getPixelFormat();
				type = image->getDataType();

				std::optional<HeightMapElevationBounds> thisTileBounds = getHeightMapElevationBounds(*image);
				if (thisTileBounds)
				{
					if (!bounds)
					{
						bounds = thisTileBounds;
					}
					else
					{
						expand(*bounds, *thisTileBounds);
					}
				}

				std::optional<HeightMapElevationRerange> thisTileRerange = getHeightMapElevationRerange(*image);
				if (thisTileRerange)
				{
					if (!rerange)
					{
						rerange = thisTileRerange;
					}
					else
					{
						if (*rerange != *thisTileRerange)
						{
							throw std::runtime_error("Source tiles have inconsistant elevation ranges");
						}
					}
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
	osg::ref_ptr<osg::Image> composite(new osg::Image);
	composite->allocateImage(256, 256, 1, pixelFormat, type);

	if (isHeightMapDataFormat(*composite))
	{
		composite->setInternalTextureFormat(getHeightMapInternalTextureFormat());
	}

	osg::Vec2d size = keyBounds.size();
	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			osg::Vec2d latLon = keyBounds.minimum + osg::Vec2d(size.x() * (double(y) + 0.5) / 256.0, size.y() * (double(x) + 0.5) / 256.0);
			osg::Vec2f srcXy = latLongToPixelXY(latLon.x(), latLon.y(), quadTreeTileKey.level);

			auto it = tiles.find(pixelXYToTileXY(toVec2i(srcXy)));
			if (it != tiles.end())
			{
				const osg::Image& src = *it->second;
				composite->setColor(getColorBilinear(src, osg::Vec2f(fmodf(srcXy.x(), 256.f), 255.f - fmodf(srcXy.y(), 256.f))), x, y);
			}
		}
	}

	if (bounds)
	{
		setHeightMapElevationBounds(*composite, *bounds);
	}

	if (rerange)
	{
		setHeightMapElevationRerange(*composite, *rerange);
	}

	return composite;
}

} // namespace vis
} // namespace skybolt
