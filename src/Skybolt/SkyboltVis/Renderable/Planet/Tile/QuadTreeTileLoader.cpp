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
	progressCallback(new TileProgressCallback),
	dataPtr(new OsgTilePtr)
{
}

AsyncQuadTreeTile::~AsyncQuadTreeTile()
{
	progressCallback->cancel();
}

const OsgTile* AsyncQuadTreeTile::getData() const
{
	assert(dataPtr);
	return dataPtr->get();
}

QuadTreeTileLoader::QuadTreeTileLoader(const AsyncTileLoaderPtr& asyncTileLoader, PlanetSubdivisionPredicate* predicate, skybolt::Listenable<PlanetSurfaceListener>* listener) :
	mAsyncTileLoader(asyncTileLoader),
	mSubdivisionRequiredPredicate(predicate),
	mListener(listener)
{
	assert(mAsyncTileLoader);

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
	mWorldTree->leftTree.merge(mWorldTree->leftTree.getRoot());
	mWorldTree->leftTree.getRoot().progressCallback->cancel();
	mWorldTree->rightTree.merge(mWorldTree->rightTree.getRoot());
	mWorldTree->rightTree.getRoot().progressCallback->cancel();

	mAsyncTileLoader.reset();

	for (size_t i = 0; i < mLoadQueue.size(); ++i)
		CALL_LISTENERS_ON_OBJECT(tileLoadCanceled(), mListener);
}

void QuadTreeTileLoader::update(std::vector<AsyncQuadTreeTile*>& addedTiles, std::vector<QuadTreeTileKey>& removedTiles)
{
	traveseToLoadAndUnload(mWorldTree->leftTree, mWorldTree->leftTree.getRoot(), false);
	traveseToLoadAndUnload(mWorldTree->rightTree, mWorldTree->rightTree.getRoot(), false);

	mAsyncTileLoader->update();

	for (auto it = mLoadQueue.begin(); it != mLoadQueue.end();)
	{
		const LoadRequest& request = *it;
		if (request.progressCallback->state != TileProgressCallback::State::Loading)
		{
			if (request.progressCallback->state == TileProgressCallback::State::Loaded)
			{
				CALL_LISTENERS_ON_OBJECT(tileLoaded(), mListener);
			}
			else // tile no longer loading and not loaded. Must have been cancelled.
			{
				CALL_LISTENERS_ON_OBJECT(tileLoadCanceled(), mListener);
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
	bool sufficient = !(*mSubdivisionRequiredPredicate)(tile.bounds, tile.key);

	// Copy loaded state so that it doesn't change from another thread during this traversal logic, which might violate assumptions
	bool loaded = tile.progressCallback->state == TileProgressCallback::State::Loaded;

	// Load tile if it should be loaded and isn't currently
	bool shouldBeLoaded = !parentIsSufficient;
	if (shouldBeLoaded)
	{
		if (tile.progressCallback->state == TileProgressCallback::State::NotLoaded)
		{
			loadTile(tile);
		}
	}
	else if (!loaded) // cancel loading if tile should not be loaded
	{
		tile.progressCallback->cancel();
	}

	if (loaded)
	{
		// If tile is loaded and not sufficiently detailed, subdive it
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
	if (mLoadQueue.size() > 32)
		return;

	tile.progressCallback = std::make_shared<TileProgressCallback>();
	Box2d latLonBounds(math::vec2SwapComponents(tile.bounds.minimum), math::vec2SwapComponents(tile.bounds.maximum));
	mAsyncTileLoader->load(tile.key, latLonBounds, tile.dataPtr, tile.progressCallback);
	CALL_LISTENERS_ON_OBJECT(tileLoadRequested(), mListener);
	mLoadQueue.push_back({ tile.progressCallback });
}

} // namespace vis
} // namespace skybolt
