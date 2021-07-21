/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Renderable/Planet/Tile/AsyncTileLoader.h>
#include <SkyboltVis/Renderable/Planet/Tile/QuadTreeTileLoader.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileImagesLoader.h>

using namespace skybolt;
using namespace skybolt::vis;

class DummyAsyncTileLoader : public AsyncTileLoader
{
public:
	~DummyAsyncTileLoader() override = default;

	void load(const skybolt::QuadTreeTileKey& key, const TileImagesPtrPtr& result, const ProgressCallbackPtr& progress) override
	{
		requests.push_back( { key, result, progress } );
	}

	void waitForLoads() override
	{
	}

	void update() override
	{
	}

	struct Request
	{
		skybolt::QuadTreeTileKey key;
		TileImagesPtrPtr result;
		ProgressCallbackPtr progress;
	};

	std::vector<Request> requests;
};

class DummyQuadTreeSubdivisionPredicate : public QuadTreeSubdivisionPredicate
{
public:
	~DummyQuadTreeSubdivisionPredicate() override = default;

	bool operator()(const Box2d& bounds, const QuadTreeTileKey& key) override
	{
		return key.level < maxSubdivisionLevel;
	}
	int maxSubdivisionLevel = 0;
};

struct DummyTileImages : public TileImages
{
	~DummyTileImages() = default;
};

static void loadAllTiles(const std::vector<DummyAsyncTileLoader::Request>& requests)
{
	for (const auto& request : requests)
	{
		*request.result = std::make_shared<DummyTileImages>();
		request.progress->state = TileProgressCallback::State::Loaded;
	}
}

SCENARIO("Test QuadTreeTileLoader requests load of tiles that pass predicate")
{
	GIVEN("A QuadTreeTileLoader")
	{
		auto asyncTileLoader = std::make_shared<DummyAsyncTileLoader>();
		auto predicate = std::make_shared<DummyQuadTreeSubdivisionPredicate>();
		QuadTreeTileLoader loader(asyncTileLoader, predicate);
		auto loadedTree = loader.getLoadedTree();

		WHEN("First update")
		{
			predicate->maxSubdivisionLevel = 1;
			loader.update();

			THEN("First level load requested")
			{
				CHECK(asyncTileLoader->requests.size() == 2);
			}

			AND_WHEN("Second update before second level loaded")
			{
				loader.update();

				THEN("No more requests because first level not loaded")
				{
					CHECK(asyncTileLoader->requests.size() == 2);
					CHECK(!loadedTree->leftTree.getRoot().hasChildren());
				}

				AND_WHEN("First level loaded")
				{
					loadAllTiles(asyncTileLoader->requests);
					loader.update();

					THEN("Second level load requested")
					{
						CHECK(asyncTileLoader->requests.size() == 10);
					}

					AND_WHEN("Second level loaded")
					{
						loadAllTiles(asyncTileLoader->requests);
						loader.update();

						THEN("Loaded tree contains the second level")
						{
							CHECK(loadedTree->leftTree.getRoot().hasChildren());
						}

						AND_WHEN("Second level mo longer passes predicate")
						{
							predicate->maxSubdivisionLevel = 0;
							loader.update();

							THEN("Second level unloaded")
							{
								CHECK(!loadedTree->leftTree.getRoot().hasChildren());
							}
						}
					}
				}
			}
		}
	}
}

static std::shared_ptr<QuadTreeTileLoader::LoadedTileTree> createTree()
{
	Box2d leftBounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(0, math::halfPiD()));
	Box2d rightBounds(osg::Vec2d(0, -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));
	return std::make_shared<QuadTreeTileLoader::LoadedTileTree>(
		[](const QuadTreeTileKey& key, const Box2T<osg::Vec2d>& bounds) {
		auto tile = std::make_unique<QuadTreeTileLoader::LoadedTile>();
		tile->key = key;
		tile->bounds = bounds;
		tile->images = std::make_shared<DummyTileImages>();
		return tile;
	},
		QuadTreeTileKey(0, 0, 0), leftBounds, QuadTreeTileKey(0, 1, 0), rightBounds);
}

TEST_CASE("Find all leaf tiles of a tree")
{
	auto tree = createTree();
	TileKeyImagesMap result;
	std::optional<int> maxLevel = {};

	SECTION("No subdivision and no images")
	{
		tree->leftTree.getRoot().images = nullptr;
		tree->rightTree.getRoot().images = nullptr;
		findLeafTiles(*tree, result, maxLevel);
		CHECK(result.size() == 0);
	}
	SECTION("No subdivision with images")
	{
		findLeafTiles(*tree, result, maxLevel);
		CHECK(result.size() == 2);
	}
	SECTION("One subdivision level with images")
	{
		tree->leftTree.subdivide(tree->leftTree.getRoot());
		tree->rightTree.subdivide(tree->rightTree.getRoot());
		findLeafTiles(*tree, result, maxLevel);
		CHECK(result.size() == 8);
	}
	SECTION("Find is limited to max level, with one subdivision level with images")
	{
		tree->leftTree.subdivide(tree->leftTree.getRoot());
		tree->rightTree.subdivide(tree->rightTree.getRoot());
		maxLevel = 0;
		findLeafTiles(*tree, result, maxLevel);
		CHECK(result.size() == 2);
	}
}

TEST_CASE("Find tiles added and removed from tree")
{
	TileKeyImagesMap previousTiles;
	TileKeyImagesMap currentTiles;

	SECTION("No tiles")
	{
		TileKeyImagesMap addedTiles;
		std::set<QuadTreeTileKey> removedTiles;
		findAddedAndRemovedTiles(previousTiles, currentTiles, addedTiles, removedTiles);
		CHECK(addedTiles.empty());
		CHECK(removedTiles.empty());
	}

	SECTION("Tile added")
	{
		currentTiles = {{QuadTreeTileKey(0,0,0), std::make_shared<DummyTileImages>()} };

		TileKeyImagesMap addedTiles;
		std::set<QuadTreeTileKey> removedTiles;
		findAddedAndRemovedTiles(previousTiles, currentTiles, addedTiles, removedTiles);
		CHECK(addedTiles == currentTiles);
		CHECK(removedTiles.empty());
	}

	SECTION("No tiles added or removed")
	{
		previousTiles = {{QuadTreeTileKey(0,0,0), std::make_shared<DummyTileImages>()}};
		currentTiles = previousTiles;

		TileKeyImagesMap addedTiles;
		std::set<QuadTreeTileKey> removedTiles;
		findAddedAndRemovedTiles(previousTiles, currentTiles, addedTiles, removedTiles);
		CHECK(addedTiles.empty());
		CHECK(removedTiles.empty());
	}

	SECTION("Tile removed")
	{
		previousTiles = {{QuadTreeTileKey(0,0,0), std::make_shared<DummyTileImages>()}};

		TileKeyImagesMap addedTiles;
		std::set<QuadTreeTileKey> removedTiles;
		findAddedAndRemovedTiles(previousTiles, currentTiles, addedTiles, removedTiles);
		CHECK(addedTiles.empty());
		CHECK(removedTiles == std::set<QuadTreeTileKey>({ QuadTreeTileKey(0, 0, 0) }));
	}
}