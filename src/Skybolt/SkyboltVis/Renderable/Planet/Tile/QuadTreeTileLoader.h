/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltCommon/Listenable.h>
#include "SkyboltCommon/Math/Box2.h"
#include <SkyboltCommon/Math/QuadTree.h>

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
	// NOTE: we use a functor struct to represent the predicate rather than a std::function because the client may want to modify predicate
	// state after providing the predicate to the QuadTreeTileLoader and std::function would require copying the predicate which would break this.

	virtual ~QuadTreeSubdivisionPredicate() = default;

//! @returns true if the tile with the given key should be subdivded.
//! @param images specifies the tile's images if the tile is loaded, otherwise nullptr. This is useful if the subdivision decision
//!     is based on image content, for example how close the camera is to elevations stored in a height map image.
	virtual bool operator()(const Box2d& bounds, const QuadTreeTileKey& key, const TileImages* images) = 0;
};

using QuadTreeSubdivisionPredicatePtr = std::shared_ptr<QuadTreeSubdivisionPredicate>;

struct QuadTreeTileLoadPredicate
{
	virtual ~QuadTreeTileLoadPredicate() = default;

	//! @returns true if the tile with the given key should be loaded.
	virtual bool operator()(const Box2d& bounds, const QuadTreeTileKey& key) = 0;
};

using QuadTreeTileLoadPredicatePtr = std::shared_ptr<QuadTreeTileLoadPredicate>;

struct QuadTreeTileAlwaysLoad : public QuadTreeTileLoadPredicate
{
	~QuadTreeTileAlwaysLoad() override = default;
	bool operator()(const Box2d& bounds, const QuadTreeTileKey& key) override { return true; }
};

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
	QuadTreeTileLoader(AsyncTileLoaderPtr asyncTileLoader, QuadTreeSubdivisionPredicatePtr subdivisionPredicate, QuadTreeTileLoadPredicatePtr loadPredicate = std::make_shared<QuadTreeTileAlwaysLoad>());

	~QuadTreeTileLoader();

	void update();

	bool isLoading() const { return !mLoadQueue.empty(); }

	struct LoadedTile : public skybolt::QuadTreeTile<glm::dvec2, LoadedTile>
	{
		TileImagesPtr images;
	};
	typedef skybolt::DiQuadTree<struct LoadedTile> LoadedTileTree;
	typedef std::shared_ptr<LoadedTileTree> LoadedTileTreePtr;

	LoadedTileTreePtr getLoadedTree() const { return mLoadedTree; }

private:
	void traveseToLoadAndUnload(skybolt::QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile);
	
	//! Populates destTree with only loaded (or skipped for loading) tiles from srcTile. Tiles will be added to destTree if they are loaded in srcTile, otherwise they will be removed from destTree.
	//! This is used to create a view of the tree which only contains tiles with all siblings loaded, i.e. no missing tiles that are still loading.
	void populateLoadedTree(AsyncQuadTreeTile& srcTile, skybolt::QuadTree<LoadedTile>& destTree, LoadedTile& destTile) const;

	void loadTile(AsyncQuadTreeTile& tile);

private:
	typedef skybolt::DiQuadTree<struct AsyncQuadTreeTile> AsyncQuadTree;
	typedef std::shared_ptr<AsyncQuadTree> AsyncTileTreePtr;

	AsyncTileLoaderPtr mAsyncTileLoader;
	QuadTreeSubdivisionPredicatePtr mSubdivisionPredicate;
	QuadTreeTileLoadPredicatePtr mTileLoadPredicate;
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
