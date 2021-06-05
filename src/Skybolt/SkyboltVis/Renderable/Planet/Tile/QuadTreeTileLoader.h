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

namespace skybolt {
namespace vis {

struct AsyncQuadTreeTile : public skybolt::QuadTreeTile<osg::Vec2d, AsyncQuadTreeTile>
{
	AsyncQuadTreeTile();

	~AsyncQuadTreeTile();

	//! @returns null if the tile is not visible (for example, if the descendent leaves of this tile are visible instead)
	const TileImages* getData() const;

	//! The stored as shared_pointer to ensure threadsafe modification from nullptr to a valid object by the producer.
	std::shared_ptr<TileImagesPtr> dataPtr;

	enum class State
	{
		NotLoaded, // not loaded and not in the loading queue
		Loading, // in the loading queue
		Loaded // loaded
	};

	State getState() const;

	void requestCancelLoad();

	ProgressCallbackPtr progressCallback; //!< nullptr if load has not been initiated
};

typedef skybolt::DiQuadTree<struct AsyncQuadTreeTile> WorldTileTree;
typedef std::shared_ptr<WorldTileTree> WorldTileTreePtr;

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

	virtual bool operator()(const Box2d& bounds, const QuadTreeTileKey& key) = 0;
};

using QuadTreeSubdivisionPredicatePtr = std::shared_ptr<QuadTreeSubdivisionPredicate>;

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

	void update(std::vector<AsyncQuadTreeTile*>& addedTiles, std::vector<skybolt::QuadTreeTileKey>& removedTiles);

	WorldTileTreePtr getTree() const { return mWorldTree; }

private:
	void traveseToLoadAndUnload(skybolt::QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile, bool parentIsSufficient);

	void traveseToCollectVisibleTiles(skybolt::QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile, std::set<skybolt::QuadTreeTileKey>& tiles, std::vector<AsyncQuadTreeTile*>& addedTiles);

	void loadTile(AsyncQuadTreeTile& tile);

private:
	AsyncTileLoaderPtr mAsyncTileLoader;
	QuadTreeSubdivisionPredicatePtr mSubdivisionPredicate;
	WorldTileTreePtr mWorldTree;

	struct LoadRequest
	{
		ProgressCallbackPtr progressCallback;
	};

	std::vector<LoadRequest> mLoadQueue;
	std::set<skybolt::QuadTreeTileKey> mVisibleTiles;
};

} // namespace vis
} // namespace skybolt
