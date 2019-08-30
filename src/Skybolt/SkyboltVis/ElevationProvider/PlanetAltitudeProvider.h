/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltCommon/LruCacheMap.h>
#include <SkyboltCommon/LruCacheSet.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Image>
#include <boost/optional.hpp>
#include <string>

namespace skybolt {
namespace vis {

// Vec2 order is lon, lat
struct LatLonVec2Adapter : public sim::LatLon
{
	typedef double value_type;

	LatLonVec2Adapter() {}

	LatLonVec2Adapter(double lon, double lat) : sim::LatLon(lat, lon) {}
	LatLonVec2Adapter(const sim::LatLon& latLon) : sim::LatLon(latLon) {}

	const double& x() const
	{
		return lon;
	}

	const double& y() const
	{
		return lat;
	}

	double& x()
	{
		return lon;
	}

	double& y()
	{
		return lat;
	}

	double operator[] (int i) const { return (&lat)[i]; }
	double& operator[] (int i) { return (&lat)[i]; }

	LatLonVec2Adapter operator+(const LatLonVec2Adapter& latLon) const
	{
		return LatLonVec2Adapter(lon + latLon.lon, lat + latLon.lat);
	}

	LatLonVec2Adapter operator-(const LatLonVec2Adapter& latLon) const
	{
		return LatLonVec2Adapter(lon - latLon.lon, lat - latLon.lat);
	}

	LatLonVec2Adapter operator*(double s) const
	{
		return LatLonVec2Adapter(lon * s, lat * s);
	}

	LatLonVec2Adapter operator/(double s) const
	{
		return LatLonVec2Adapter(lon / s, lat / s);
	}
};

class PlanetAltitudeProvider
{
public:
	PlanetAltitudeProvider(const TileSourcePtr& tileSource, int maxLod);

	//! Altitude above sea level, positive is up.
	double getAltitude(const sim::LatLon& position) const;

	typedef skybolt::Box2T<LatLonVec2Adapter> LatLonBounds;

private:
	struct TileImage
	{
		QuadTreeTileKey key;
		osg::ref_ptr<osg::Image> image;
	};

	boost::optional<TileImage> findHighestLodTileIntersectingPoint(const sim::LatLon& position) const;

private:
	const TileSourcePtr mTileSource;
	const int mMaxLod;

	// Note: By design, the TileImage key does not necessarily equal the cache key,
	// because lower lods will be used (and put in the cache) if the requested lod (the cache key)
	// is unavailable.
	mutable LruCacheMap<QuadTreeTileKey, TileImage> mTileImageCache;
};


} // namespace vis
} // namespace skybolt