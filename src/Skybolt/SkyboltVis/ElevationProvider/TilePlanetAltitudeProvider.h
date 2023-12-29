/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltSim/PlanetAltitudeProvider.h>
#include <SkyboltCommon/LruCacheMap.h>
#include <SkyboltCommon/LruCacheSet.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Image>
#include <px_sched/px_sched.h>
#include <optional>

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

//! Blocks until tile at maxLod is is loaded, then returns result
class BlockingTilePlanetAltitudeProvider : public sim::PlanetAltitudeProvider
{
public:
	BlockingTilePlanetAltitudeProvider(const TileSourcePtr& tileSource, int maxLod);

	//! @ThreadSafe
	AltitudeResult getAltitude(const sim::LatLon& position) const override;

	typedef skybolt::Box2T<LatLonVec2Adapter> LatLonBounds;

protected:
	struct TileImage
	{
		QuadTreeTileKey key;
		osg::ref_ptr<osg::Image> image;
	};

	std::optional<TileImage> findTile(const QuadTreeTileKey& key) const;

	std::optional<TileImage> loadTile(const QuadTreeTileKey& key) const;

	void addTileToCache(const TileImage& image, const QuadTreeTileKey& key) const;

protected:
	const TileSourcePtr mTileSource;
	const int mMaxLod;

	mutable LruCacheMap<QuadTreeTileKey, TileImage> mTileImageCache;
	mutable std::mutex mTileImageCacheMutex;
};

//! Immediately returns result from an already loaded tile at the highest available LOD, and schedules a background task to load higher LOD levels if requred
class NonBlockingTilePlanetAltitudeProvider : public BlockingTilePlanetAltitudeProvider
{
public:
	NonBlockingTilePlanetAltitudeProvider(px_sched::Scheduler* scheduler, const TileSourcePtr& tileSource, int maxLod);

	//! @ThreadSafe
	AltitudeResult getAltitude(const sim::LatLon& position) const override;

protected:
	void requestLoadTileAndAddToCache(const QuadTreeTileKey& key) const;

protected:
	mutable px_sched::Sync mLoadingTaskSync;
	px_sched::Scheduler* mScheduler;
};

} // namespace vis
} // namespace skybolt