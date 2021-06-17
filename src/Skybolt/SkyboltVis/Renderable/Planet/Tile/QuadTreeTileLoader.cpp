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

	QuadTree<AsyncQuadTreeTile>::TileCreator tileCreator = [](const QuadTreeTileKey& key, const Box2T<typename AsyncQuadTreeTile::VectorType>& bounds)
	{
		AsyncQuadTreeTile* tile = new AsyncQuadTreeTile;
		tile->key = key;
		tile->bounds = bounds;
		return std::unique_ptr<AsyncQuadTreeTile>(tile);
	};

	Box2d leftBounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(0, math::halfPiD()));
	Box2d rightBounds(osg::Vec2d(0, -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));
	mWorldTree.reset(new WorldTileTree(tileCreator, QuadTreeTileKey(0, 0, 0), leftBounds, QuadTreeTileKey(0, 1, 0), rightBounds));
}

QuadTreeTileLoader::~QuadTreeTileLoader()
{
	// Unload all tiles
	auto& leftRoot = mWorldTree->leftTree.getRoot();
	mWorldTree->leftTree.merge(leftRoot);
	leftRoot.requestCancelLoad();

	auto& rightRoot = mWorldTree->rightTree.getRoot();
	mWorldTree->rightTree.merge(rightRoot);
	rightRoot.requestCancelLoad();

	mAsyncTileLoader.reset();

	for (size_t i = 0; i < mLoadQueue.size(); ++i)
		CALL_LISTENERS(tileLoadCanceled());
}

void QuadTreeTileLoader::update(std::vector<AsyncQuadTreeTile*>& addedTiles, std::vector<QuadTreeTileKey>& removedTiles)
{
	traveseToLoadAndUnload(mWorldTree->leftTree, mWorldTree->leftTree.getRoot(), false);
	traveseToLoadAndUnload(mWorldTree->rightTree, mWorldTree->rightTree.getRoot(), false);

	mAsyncTileLoader->update();

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

	std::set<QuadTreeTileKey> tiles;
	traveseToCollectVisibleTiles(mWorldTree->leftTree, mWorldTree->leftTree.getRoot(), tiles, addedTiles);
	traveseToCollectVisibleTiles(mWorldTree->rightTree, mWorldTree->rightTree.getRoot(), tiles, addedTiles);

	for (const QuadTreeTileKey& key : mVisibleTiles)
	{
		if (tiles.find(key) == tiles.end())
		{
			removedTiles.push_back(key);
		}
	}

	std::swap(tiles, mVisibleTiles);
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

void QuadTreeTileLoader::traveseToCollectVisibleTiles(QuadTree<AsyncQuadTreeTile>& tree, AsyncQuadTreeTile& tile, std::set<QuadTreeTileKey>& tiles, std::vector<AsyncQuadTreeTile*>& addedTiles)
{
	if (tile.getData())
	{
		// Determine if tile is a leaf node.
		// A tile is a leaf node if it has no children, or one or more children do not have loaded data.
		bool leaf = true;
		if (tile.hasChildren())
		{
			bool allChildrenHaveData = true;
			for (int i = 0; i < 4; ++i)
			{
				AsyncQuadTreeTile& child = *tile.children[i];
				if (!child.getData())
				{
					allChildrenHaveData = false;
					break;
				}
			}
			if (allChildrenHaveData)
			{
				leaf = false;
			}
		}
		
		// If tile is a laf node, make sure it's in the visible set
		if (leaf)
		{
			auto it = mVisibleTiles.find(tile.key);
			if (it == mVisibleTiles.end())
			{
				addedTiles.push_back(&tile);
			}
			tiles.insert(tile.key);
		}
		else if (tile.hasChildren()) // tile is not a leaf-node and has children
		{
			// Continue traversal to children
			for (int i = 0; i < 4; ++i)
			{
				AsyncQuadTreeTile& child = *tile.children[i];
				traveseToCollectVisibleTiles(tree, child, tiles, addedTiles);
			}
		}
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

} // namespace vis
} // namespace skybolt
