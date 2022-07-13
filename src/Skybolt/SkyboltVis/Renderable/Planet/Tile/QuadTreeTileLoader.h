/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltCommon/Listenable.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Vec2d>

#include <assert.h>
#include <set>
#include <vector>

namespace skybolt {
namespace vis {

struct QuadTreeTileLoaderListener
{
	virtual ~QuadTreeTileLoaderListener() = default;
	virtual void tileLoadRequested() {}
	virtual void tileLoaded() {}
	virtual void tileLoadCanceled() {}
};

struct QuadTreeSubdivisionPredicate
{
	virtual ~QuadTreeSubdivisionPredicate() = default;

	//! Returns true if the tile with the given key should be subdivded.
	//! @param images specifies the tile's images, which are useful if the subdivision decision is based on image content,
	//!        for example how close the camera is to elevations stored in a height map image.
	virtual bool operator()(const Box2d& bounds, const QuadTreeTileKey& key, const TileImages& images) = 0;
};

using QuadTreeSubdivisionPredicatePtr = std::shared_ptr<QuadTreeSubdivisionPredicate>;

struct AsyncQuadTreeTile;

//! QuadTreeTileLoader loads a quadtree of tiles to satisfy a predicate governing whether a given tile is of sufficient resolution.
//! While a tile is of sufficient resolution, child tiles will not be loaded.
//! If a tile is of insufficient resolution, its children will be loaded.
//! This strategy causes tiles to appear in sequential increments of detail, i.e first level 0, then level 1 etc.
//! This was found to give the appearance of faster map loading because the 'next-best' resolution tile is available
//! while the best resolution tile is still loading.
class QuadTreeTileLoader : public skybolt::Listenable<QuadTreeTileLoaderListener>
{
public:
	QuadTreeTileLoader(const AsyncTileLoaderPtr& asyncTileLoader, const QuadTreeSubdivisionPredicatePtr& predicate);

	~QuadTreeTileLoader();

	void update();

	struct LoadedTile : public skybolt::QuadTreeTile<osg::Vec2d, LoadedTile>
	{
		TileImagesPtr images;
	};
	typedef skybolt::DiQuadTree<struct LoadedTile> LoadedTileTree;
	typedef std::shared_ptr<LoadedTileTree> LoadedTileTreePtr;

	LoadedTileTreePtr getLoadedTree() const { return mLoadedTree; }

private:
	void traveseToLoadAndUnload(skybolt::QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile);
	
	void populateLoadedTree(AsyncQuadTreeTile& srcTile, skybolt::QuadTree<LoadedTile>& destTree, LoadedTile& destTile) const;

	void loadTile(AsyncQuadTreeTile& tile);

private:
	typedef skybolt::DiQuadTree<struct AsyncQuadTreeTile> AsyncQuadTree;
	typedef std::shared_ptr<AsyncQuadTree> AsyncTileTreePtr;

	AsyncTileLoaderPtr mAsyncTileLoader;
	QuadTreeSubdivisionPredicatePtr mSubdivisionPredicate;
	AsyncTileTreePtr mAsyncTree;
	LoadedTileTreePtr mLoadedTree;

	struct LoadRequest
	{
		ProgressCallbackPtr progressCallback;
	};

	std::vector<LoadRequest> mLoadQueue;
};

using TileKeyImagesMap = std::map<QuadTreeTileKey, TileImagesPtr>;

void findLeafTiles(const QuadTreeTileLoader::LoadedTile& tile, TileKeyImagesMap& result, std::optional<int> maxLevel = std::nullopt);
void findLeafTiles(const QuadTreeTileLoader::LoadedTileTree& tree, TileKeyImagesMap& result, std::optional<int> maxLevel = std::nullopt);

void findAddedAndRemovedTiles(const TileKeyImagesMap& previousTiles, const TileKeyImagesMap& currentTiles,
	TileKeyImagesMap& addedTiles, std::set<QuadTreeTileKey>& removedTiles);

} // namespace vis
} // namespace skybolt
