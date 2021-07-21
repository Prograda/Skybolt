/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QuadTreeTileLoader.h"
#include "AsyncTileLoader.h"
#include "PlanetSubdivisionPredicate.h"
#include "SkyboltVis/OsgMathHelpers.h"

#include <SkyboltCommon/Listenable.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

std::ostream& operator<<(std::ostream& s, const osg::Vec3f& v)
{
	s << "(" << v.x() << " " << v.y() << " " << v.z() << ")";
	return s;
}

int maxHeightMapTileLevel = 10;

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

AsyncQuadTreeTile::AsyncQuadTreeTile() :
	dataPtr(new TileImagesPtr)
{
}

AsyncQuadTreeTile::~AsyncQuadTreeTile()
{
	if (progressCallback)
	{
		progressCallback->requestCancel();
	}
}

const TileImages* AsyncQuadTreeTile::getData() const
{
	assert(dataPtr);
	return dataPtr->get();
}

AsyncQuadTreeTile::State AsyncQuadTreeTile::getState() const
{
	if (progressCallback)
	{
		auto progressState = progressCallback->state.load();
		if (progressState == TileProgressCallback::State::Loaded)
		{
			return State::Loaded;
		}
		else if (progressState == TileProgressCallback::State::Loading)
		{
			return State::Loading;
		}
	}
	return State::NotLoaded;
}

void AsyncQuadTreeTile::requestCancelLoad()
{
	if (progressCallback)
	{
		progressCallback->requestCancel();
	}
}

QuadTreeTileLoader::QuadTreeTileLoader(const AsyncTileLoaderPtr& asyncTileLoader, const QuadTreeSubdivisionPredicatePtr& predicate) :
	mAsyncTileLoader(asyncTileLoader),
	mSubdivisionPredicate(predicate)
{
	assert(mAsyncTileLoader);
	assert(mSubdivisionPredicate);

	Box2d leftBounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(0, math::halfPiD()));
	Box2d rightBounds(osg::Vec2d(0, -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));
	mAsyncTree = std::make_shared<AsyncQuadTree>(createTileT<AsyncQuadTreeTile, AsyncQuadTreeTile::VectorType>, QuadTreeTileKey(0, 0, 0), leftBounds, QuadTreeTileKey(0, 1, 0), rightBounds);
	mLoadedTree = std::make_shared<LoadedTileTree>(createTileT<LoadedTile, LoadedTile::VectorType>, QuadTreeTileKey(0, 0, 0), leftBounds, QuadTreeTileKey(0, 1, 0), rightBounds);
}

QuadTreeTileLoader::~QuadTreeTileLoader()
{
	// Unload all tiles
	auto& leftRoot = mAsyncTree->leftTree.getRoot();
	mAsyncTree->leftTree.merge(leftRoot);
	leftRoot.requestCancelLoad();

	auto& rightRoot = mAsyncTree->rightTree.getRoot();
	mAsyncTree->rightTree.merge(rightRoot);
	rightRoot.requestCancelLoad();

	mAsyncTileLoader.reset();

	for (size_t i = 0; i < mLoadQueue.size(); ++i)
		CALL_LISTENERS(tileLoadCanceled());
}

void QuadTreeTileLoader::update()
{
	// Process results from previous load requests
	for (auto it = mLoadQueue.begin(); it != mLoadQueue.end();)
	{
		const LoadRequest& request = *it;
		assert(request.progressCallback); // must exist if tile is in queue
		if (request.progressCallback->state != TileProgressCallback::State::Loading)
		{
			if (request.progressCallback->state == TileProgressCallback::State::Loaded)
			{
				CALL_LISTENERS(tileLoaded());
			}
			else // tile no longer loading and not loaded. Must have either been cancelled or failed
			{
				CALL_LISTENERS(tileLoadCanceled());
			}
			it = mLoadQueue.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Issue new load/unloads
	traveseToLoadAndUnload(mAsyncTree->leftTree, mAsyncTree->leftTree.getRoot(), false);
	traveseToLoadAndUnload(mAsyncTree->rightTree, mAsyncTree->rightTree.getRoot(), false);

	// Tick the async loader
	mAsyncTileLoader->update();

	// Copy loaded tiles from the async tree to the loaded tree
	{
		populateLoadedTree(mAsyncTree->leftTree.getRoot(), mLoadedTree->leftTree, mLoadedTree->leftTree.getRoot());
		populateLoadedTree(mAsyncTree->rightTree.getRoot(), mLoadedTree->rightTree, mLoadedTree->rightTree.getRoot());
	}
}

void QuadTreeTileLoader::traveseToLoadAndUnload(QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile, bool parentIsSufficient)
{
	bool sufficient = !(*mSubdivisionPredicate)(tile.bounds, tile.key);

	auto state = tile.getState();

	// Load tile if it should be loaded and isn't currently
	bool shouldBeLoaded = !parentIsSufficient;
	if (shouldBeLoaded)
	{
		if (state == AsyncQuadTreeTile::State::NotLoaded)
		{
			loadTile(tile);
		}
	}
	else if (state != AsyncQuadTreeTile::State::Loaded) // cancel loading if tile should not be loaded
	{
		tile.requestCancelLoad();
	}

	if (state == AsyncQuadTreeTile::State::Loaded)
	{
		// If tile is loaded and not sufficiently detailed, subdivide it
		if (!sufficient)
		{
			if (!tile.hasChildren())
				tree.subdivide(tile);
		}
		else // tile is loaded and detailed enough. Merge any children.
		{
			if (tile.hasChildren())
				tree.merge(tile);
		}
	}


	// Continue traversing to children
	if (tile.hasChildren())
	{
		for (int i = 0; i < 4; ++i)
		{
			AsyncQuadTreeTile& child = *tile.children[i];
			traveseToLoadAndUnload(tree, child, sufficient);
		}
	}
}

void QuadTreeTileLoader::populateLoadedTree(AsyncQuadTreeTile& srcTile, skybolt::QuadTree<LoadedTile>& dstTree, LoadedTile& dstTile) const
{
	if (srcTile.getData())
	{
		dstTile.images = *srcTile.dataPtr;

		if (srcTile.hasChildren())
		{
			bool allChildrenLoaded = true;
			for (int i = 0; i < 4; ++i)
			{
				if (!srcTile.children[i]->getData())
				{
					allChildrenLoaded = false;
					break;
				}
			}

			if (allChildrenLoaded)
			{
				if (!dstTile.hasChildren())
				{
					dstTree.subdivide(dstTile);
				}

				for (int i = 0; i < 4; ++i)
				{
					populateLoadedTree(*srcTile.children[i], dstTree, *dstTile.children[i]);
				}
			}
		}
		else if (dstTile.hasChildren())
		{
			dstTree.merge(dstTile);
		}
	}
	else
	{
		dstTile.images = nullptr;
	}
}

void QuadTreeTileLoader::loadTile(AsyncQuadTreeTile& tile)
{
	assert(tile.getState() == AsyncQuadTreeTile::State::NotLoaded);

	if (mLoadQueue.size() > 32)
		return;

	tile.progressCallback = std::make_shared<TileProgressCallback>();
	mAsyncTileLoader->load(tile.key, tile.dataPtr, tile.progressCallback);
	CALL_LISTENERS(tileLoadRequested());
	mLoadQueue.push_back({ tile.progressCallback });
}

void findLeafTiles(const QuadTreeTileLoader::LoadedTile& tile, TileKeyImagesMap& result, std::optional<int> maxLevel)
{
	if ((maxLevel && tile.key.level == *maxLevel) || !tile.hasChildren())
	{
		if (tile.images)
		{
			result[tile.key] = tile.images;
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			findLeafTiles(*tile.children[i], result, maxLevel);
		}
	}
}

void findLeafTiles(const QuadTreeTileLoader::LoadedTileTree& tree, TileKeyImagesMap& result, std::optional<int> maxLevel)
{
	findLeafTiles(tree.leftTree.getRoot(), result, maxLevel);
	findLeafTiles(tree.rightTree.getRoot(), result, maxLevel);
}

void findAddedAndRemovedTiles(const TileKeyImagesMap& previousTiles, const TileKeyImagesMap& currentTiles,
	TileKeyImagesMap& addedTiles, std::set<QuadTreeTileKey>& removedTiles)
{
	for (const auto&[key, value] : currentTiles)
	{
		if (previousTiles.find(key) == previousTiles.end())
		{
			addedTiles[key] = value;
		}
	}

	for (const auto&[key, value] : previousTiles)
	{
		if (currentTiles.find(key) == currentTiles.end())
		{
			removedTiles.insert(key);
		}
	}
}

} // namespace vis
} // namespace skybolt
