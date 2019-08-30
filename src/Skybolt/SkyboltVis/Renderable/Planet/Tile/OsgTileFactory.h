/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OsgTile.h"
#include "TileImage.h"
#include "SkyboltVis/ShadowHelpers.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/Planet/Terrain.h"
#include <SkyboltCommon/LruCacheMap.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Image>
#include <map>

namespace std {
template <typename T>
struct hash<osg::ref_ptr<T>>
{
	size_t operator()(const osg::ref_ptr<T>& p) const
	{
		return std::hash<T*>()(p.get());
	}
};
}

namespace skybolt {
namespace vis {

struct OsgTileFactoryConfig
{
	px_sched::Scheduler* scheduler;
	const ShaderPrograms* programs;
	ShadowMaps shadowMaps;

	double planetRadius;
	float forestGeoVisibilityRange;
	bool hasCloudShadows;
};

class OsgTileFactory
{
public:
	//! @param textureCompiler is used to compile OpenGL textures (upload textures to GPU) as the tile is created, rather than letting OSG compile them all at once when they are added to the scene graph. This avoids stalls.
	OsgTileFactory(const OsgTileFactoryConfig& config);

	OsgTile createOsgTile(const skybolt::QuadTreeTileKey& key, const Box2d& latLonBounds, const TileImage& heightImage, osg::Image* landMaskImage,
		const TileImage& albedoImage, const TileImage& attributeImage) const;

private:
	double mPlanetRadius;
	px_sched::Scheduler* mScheduler;
	const ShaderPrograms* mPrograms;
	TerrainConfig mCommonTerrainConfig;
	float mForestGeoVisibilityRange;
	ShadowMaps mShadowMaps;
	bool mHasCloudShadows;

	using TextureCache = LruCacheMap<osg::ref_ptr<osg::Image>, osg::ref_ptr<osg::Texture2D>>;
	constexpr static size_t lruCacheSize = 256;
	mutable TextureCache mCacheHeight = TextureCache(lruCacheSize);
	mutable TextureCache mCacheNormal = TextureCache(lruCacheSize);
	mutable TextureCache mCacheLandMask = TextureCache(lruCacheSize);
	mutable TextureCache mCacheAlbedo = TextureCache(lruCacheSize);
	mutable TextureCache mCacheAttribute = TextureCache(lruCacheSize);
};

} // namespace vis
} // namespace skybolt
