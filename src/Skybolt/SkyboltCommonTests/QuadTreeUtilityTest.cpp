/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/QuadTreeUtility.h>

#include <osg/Vec2>

using namespace skybolt;

struct Tile : QuadTreeTile<osg::Vec2, Tile>
{
	bool keep = true;
};

typedef QuadTree<Tile> Tree;

Tree createTree()
{
	Box2T<osg::Vec2> bounds(osg::Vec2(0, 0), osg::Vec2(1, 1));
	return QuadTree<Tile>(createTileT<Tile, osg::Vec2>, QuadTreeTileKey(0, 0, 0), bounds);
}

TEST_CASE("visitHierarchyToKey")
{
	Tree tree = createTree();
	tree.subdivide(tree.getRoot());
	tree.subdivide(*tree.getRoot().children[2]);

	std::vector<Tile*> visitedTiles;

	visitHierarchyToKey<Tile>(tree.getRoot(), QuadTreeTileKey(1, 0, 0), [&](Tile& tile) {visitedTiles.push_back(&tile); });

	REQUIRE(visitedTiles.size() == 2);
	CHECK(visitedTiles[0] == &tree.getRoot());
	CHECK(visitedTiles[1] == tree.getRoot().children[0].get());
}

TEST_CASE("pruneTree")
{
	Tree tree = createTree();
	tree.subdivide(tree.getRoot());
	tree.subdivide(*tree.getRoot().children[0]);
	tree.subdivide(*tree.getRoot().children[2]);

	std::vector<Tile*> prunedTiles;

	auto shouldTraverse = [](const Tile& tile) {return true; };
	auto shouldKeep = [](const Tile& tile) {return tile.keep; };
	auto pruner = [&](Tile& tile) {prunedTiles.push_back(&tile); };

	// Check that no tiles are pruned when all tiles flagged as keep
	pruneTree<Tile>(tree.getRoot(), shouldTraverse, shouldKeep, pruner);

	CHECK(prunedTiles.size() == 0);

	// Check that tiles are pruned when they and their children are not flagged as keep
	tree.getRoot().keep = false;
	tree.getRoot().children[0]->keep = false;
	tree.getRoot().children[0]->children[0]->keep = false;
	tree.getRoot().children[2]->keep = false;
	tree.getRoot().children[2]->children[0]->keep = false;
	tree.getRoot().children[2]->children[1]->keep = false;
	tree.getRoot().children[2]->children[2]->keep = false;
	tree.getRoot().children[2]->children[3]->keep = false;

	prunedTiles.clear();
	pruneTree<Tile>(tree.getRoot(), shouldTraverse, shouldKeep, pruner);

	REQUIRE(prunedTiles.size() == 6);
	CHECK(prunedTiles[0] == tree.getRoot().children[0]->children[0].get());
	CHECK(prunedTiles[1] == tree.getRoot().children[2]->children[0].get());
	CHECK(prunedTiles[2] == tree.getRoot().children[2]->children[1].get());
	CHECK(prunedTiles[3] == tree.getRoot().children[2]->children[2].get());
	CHECK(prunedTiles[4] == tree.getRoot().children[2]->children[3].get());
	CHECK(prunedTiles[5] == tree.getRoot().children[2].get());
}
