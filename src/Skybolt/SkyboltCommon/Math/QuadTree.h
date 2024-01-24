/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Box2.h"
#include "MathUtility.h"
#include <boost/functional/hash.hpp>
#include <functional>
#include <assert.h>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace skybolt {

struct QuadTreeTileKey
{
	QuadTreeTileKey() : level(0), x(0), y(0) {}
	QuadTreeTileKey(int level, int x, int y) : level(level), x(x), y(y) {}

	bool operator < (const QuadTreeTileKey& r) const
	{
		return std::tie(level, x, y) < std::tie(r.level, r.x, r.y);
	}

	bool operator == (const QuadTreeTileKey& r) const
	{
		return std::tie(level, x, y) == std::tie(r.level, r.x, r.y);
	}

	int level;
	int x;
	int y;
};

QuadTreeTileKey createAncestorKey(const QuadTreeTileKey& key, int level);

template <class VecT, class DerivedT>
struct QuadTreeTile
{
	typedef VecT VectorType;
	Box2T<VecT> bounds;
	QuadTreeTileKey key;
	std::unique_ptr<DerivedT> children[4];

	bool hasChildren() const { return children[0] != nullptr; }
};


template <typename VecType>
struct DefaultTile : QuadTreeTile<VecType, DefaultTile<VecType>> {};

template <typename VecType>
std::unique_ptr<DefaultTile<VecType>> createDefaultTile(const QuadTreeTileKey& key, const Box2T<VecType>& bounds)
{
	auto tile = std::make_unique<DefaultTile<VecType>>();
	tile->key = key;
	tile->bounds = bounds;
	return tile;
};

template <typename TileType,typename  VecType>
std::unique_ptr<TileType> createTileT(const QuadTreeTileKey& key, const Box2T<VecType>& bounds)
{
	auto tile = std::make_unique<TileType>();
	tile->key = key;
	tile->bounds = bounds;
	return tile;
};

template <typename  VecType>
static Box2T<VecType> getKeyLonLatBounds(const QuadTreeTileKey& key)
{
	double yMax = double(1 << key.level);
	return Box2T<VecType>(
		VecType(-math::piD() + double(key.x) / yMax * math::piD(),
				math::halfPiD() - double(key.y + 1) / yMax * math::piD()),
		VecType(-math::piD() + double(key.x + 1) / yMax * math::piD(),
				math::halfPiD() - double(key.y) / yMax * math::piD())
	);
}

template <typename  VecType>
static Box2T<VecType> getKeyLatLonBounds(const QuadTreeTileKey& key)
{
	double yMax = double(1 << key.level);
	return Box2T<VecType>(
		VecType(math::halfPiD() - double(key.y + 1) / yMax * math::piD(),
			-math::piD() + double(key.x) / yMax * math::piD()),
		VecType(math::halfPiD() - double(key.y) / yMax * math::piD(),
			-math::piD() + double(key.x + 1) / yMax * math::piD())
		);
}

template <typename VecType>
QuadTreeTileKey getKeyAtLevelIntersectingLonLatPoint(int level, const VecType& position)
{
	double yMax = double(1 << level);
	QuadTreeTileKey key;
	key.level = level;
	key.x = int((position.x() + math::piD()) * yMax / math::piD());
	key.y = int((math::halfPiD() - position.y()) * yMax / math::piD());
	return key;
}

template <class TileT>
struct QuadTree
{
	typedef TileT TileType;

	typedef std::function <std::unique_ptr<TileT>(const QuadTreeTileKey& key, const Box2T<typename TileT::VectorType>& bounds)> TileCreator;

	QuadTree(TileCreator tileCreator, const QuadTreeTileKey& rootKey, const Box2T<typename TileT::VectorType>& rootBounds) :
		mTileCreator(tileCreator)
	{
		mRoot = mTileCreator(rootKey, rootBounds);
	}

	const TileT& getRoot() const
	{
		return *mRoot;
	}

	TileT& getRoot()
	{
		return *mRoot;
	}

	const TileT* intersectLeaf(const typename TileT::VectorType& p) const
	{
		return intersectLeaf(p, getRoot());
	}

	const TileT* intersectLeaf(const typename TileT::VectorType& p, const TileT& tile) const
	{
		if (tile.bounds.intersects(p))
		{
			if (tile.hasChildren())
			{
				for (int i = 0; i < 4; ++i)
				{
					const TileT* leaf = intersectLeaf(p, *tile.children[i]);
					if (leaf)
					{
						return leaf;
					}
				}
			}
			else
			{
				return &tile;
			}
		}
		return nullptr;
	}

	const TileT* intersect(const typename TileT::VectorType& p, std::function<bool (const TileT&)> predicate) const
	{
		return intersect(p, getRoot(), predicate);
	}

	typedef std::function<bool(const TileT&)> IntersectionPredicate;

	const TileT* intersect(const typename TileT::VectorType& p, const TileT& tile, IntersectionPredicate predicate) const
	{
		if (tile.bounds.intersects(p))
		{
			if (predicate(tile))
			{
				return &tile;
			}

			if (tile.hasChildren())
			{
				for (int i = 0; i < 4; ++i)
				{
					const TileT* leaf = intersect(p, *tile.children[i], predicate);
					if (leaf)
					{
						return leaf;
					}
				}
			}
		}
		return nullptr;
	}

	void subdivide(TileT& tile)
	{
		assert(!tile.hasChildren());

		// Subdivide
		int x = tile.key.x * 2;
		int y = tile.key.y * 2;
		int level = tile.key.level + 1;

		typename TileT::VectorType size = tile.bounds.size() / 2;
		typename TileT::VectorType center = tile.bounds.minimum + size;
		typename TileT::VectorType centerE(tile.bounds.maximum.x(), center.y());
		typename TileT::VectorType centerW(tile.bounds.minimum.x(), center.y());
		typename TileT::VectorType centerN(center.x(), tile.bounds.maximum.y());
		typename TileT::VectorType centerS(center.x(), tile.bounds.minimum.y());

		tile.children[0] = mTileCreator(QuadTreeTileKey(level, x, y), Box2T<typename TileT::VectorType>(centerW, centerN)); // north west
		tile.children[1] = mTileCreator(QuadTreeTileKey(level, x + 1, y), Box2T<typename TileT::VectorType>(center, tile.bounds.maximum)); // north east
		tile.children[2] = mTileCreator(QuadTreeTileKey(level, x, y + 1), Box2T<typename TileT::VectorType>(tile.bounds.minimum, center)); // south west
		tile.children[3] = mTileCreator(QuadTreeTileKey(level, x + 1, y + 1), Box2T<typename TileT::VectorType>(centerS, centerE)); // south east
	}

	using SubdivisionPredicate = std::function<bool(const TileT& tile)>;
	void subdivideRecursively(TileT& tile, const SubdivisionPredicate& subdivisionRequired)
	{
		if (subdivisionRequired(tile))
		{
			subdivide(tile);
			for (int i = 0; i < 4; ++i)
			{
				subdivideRecursively(*tile.children[i], subdivisionRequired);
			}
		}
	}

	void merge(TileT& tile)
	{
		for (int i = 0; i < 4; ++i)
		{
			tile.children[i] = nullptr;
		}
	}

private:
	TileCreator mTileCreator;
	std::unique_ptr<TileT> mRoot;
};

template <typename TileT>
struct DiQuadTree
{
	typedef TileT TileType;

	QuadTree<TileT> leftTree;
	QuadTree<TileT> rightTree;

	DiQuadTree(const typename QuadTree<TileT>::TileCreator& tileCreator, const QuadTreeTileKey& leftTreeKey, const Box2T<typename TileT::VectorType>& leftTreeBounds,
		const QuadTreeTileKey& rightTreeKey, const Box2T<typename TileT::VectorType>& rightTreeBounds) :
		leftTree(tileCreator, leftTreeKey, leftTreeBounds),
		rightTree(tileCreator, rightTreeKey, rightTreeBounds)
	{
	}

	const TileT* intersect(const typename TileT::VectorType& p, const typename skybolt::QuadTree<TileT>::IntersectionPredicate& predicate) const
	{
		const TileT* result = leftTree.intersect(p, predicate);
		if (!result)
		{
			result = rightTree.intersect(p, predicate);
		}
		return result;
	}
};


template <class TileT>
DiQuadTree<TileT> createGlobeQuadTree(typename QuadTree<TileT>::TileCreator tileCreator)
{
	static const Box2T<typename TileT::VectorType> leftBounds(typename TileT::VectorType(-skybolt::math::piD(), -skybolt::math::halfPiD()), typename TileT::VectorType(0, skybolt::math::halfPiD()));
	static const Box2T<typename TileT::VectorType> rightBounds(typename TileT::VectorType(0, -skybolt::math::halfPiD()), typename TileT::VectorType(skybolt::math::piD(), skybolt::math::halfPiD()));

	return DiQuadTree<TileT>(tileCreator, QuadTreeTileKey(0, 0, 0), leftBounds, QuadTreeTileKey(0, 1, 0), rightBounds);
}

template <class TreeT>
class LruCachedLeafIntersector
{
public:
	LruCachedLeafIntersector(const std::shared_ptr<TreeT>& tree, typename QuadTree<typename TreeT::TileType>::IntersectionPredicate predicate) :
		tree(tree), predicate(std::move(predicate)) {}

	//! @ThreadSafe
	const typename TreeT::TileType* intersect(const typename TreeT::TileType::VectorType& p)
	{
		const typename TreeT::TileType* tile = getLastTile();
		if (tile && tile->bounds.intersects(p))
		{
			return tile;
		}
		tile = tree->intersect(p, predicate);
		setLastTile(tile);
		return tile;
	}

	//! @ThreadSafe
	void invalidateCache()
	{
		std::unique_lock<std::shared_mutex> lock(lastTileMutex);
		lastTileMap.clear();
	}

private:
	const typename TreeT::TileType* getLastTile()
	{
		std::shared_lock<std::shared_mutex> lock(lastTileMutex);
		auto i = lastTileMap.find(std::this_thread::get_id());
		if (i != lastTileMap.end())
		{
			return i->second;
		}
		return nullptr;
	}

	void setLastTile(const typename TreeT::TileType* tile)
	{
		std::unique_lock<std::shared_mutex> lock(lastTileMutex);
		lastTileMap[std::this_thread::get_id()] = tile;
	}

private:
	const std::shared_ptr<TreeT> tree;
	typename QuadTree<typename TreeT::TileType>::IntersectionPredicate predicate;

	std::map<std::thread::id, const typename TreeT::TileType*> lastTileMap;
	std::shared_mutex lastTileMutex;
};


} // namespace skybolt

namespace std {
template <>
struct hash<skybolt::QuadTreeTileKey>
{
	size_t operator()(const skybolt::QuadTreeTileKey& k) const
	{
		std::size_t result = 0;
		boost::hash_combine(result, k.level);
		boost::hash_combine(result, k.x);
		boost::hash_combine(result, k.y);
		return result;
	}
};
} // namespace std