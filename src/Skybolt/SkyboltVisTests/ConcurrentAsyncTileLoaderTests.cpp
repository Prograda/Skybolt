/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Renderable/Planet/Tile/AsyncTileLoader.h>
#include <SkyboltVis/Renderable/Planet/Tile/ConcurrentAsyncTileLoader.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileImagesLoader.h>

using namespace skybolt;
using namespace skybolt::vis;

struct DummyTileImages : public TileImages
{
	DummyTileImages(skybolt::QuadTreeTileKey key) : key(key) {};
	skybolt::QuadTreeTileKey key;
};

class DummyTileImagesLoader : public TileImagesLoader
{
public:
	DummyTileImagesLoader() : TileImagesLoader(1) {}
	~DummyTileImagesLoader() override = default;

	TileImagesPtr load(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const
	{
		while (!doLoad)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1ms);
		}
		if (cancelSupplier())
		{
			return nullptr;
		}
		return std::make_shared<DummyTileImages>(key);
	}

	std::atomic_bool doLoad = false;
};

TEST_CASE("Test tile loads on background thread")
{
	auto imagesLoader = std::make_shared<DummyTileImagesLoader>();

	px_sched::Scheduler scheduler;
	scheduler.init();

	auto progressCallback = std::make_shared<TileProgressCallback>();
	QuadTreeTileKey key(0, 0, 0);
	auto result = std::make_shared<TileImagesPtr>();

	ConcurrentAsyncTileLoader loader(imagesLoader, &scheduler);
	loader.load(key, result, progressCallback);
	
	CHECK(progressCallback->state == TileProgressCallback::State::Loading);
	CHECK(!result->get());

	imagesLoader->doLoad = true;
	loader.waitForLoads();
	loader.update();

	CHECK(progressCallback->state == TileProgressCallback::State::Loaded);
	CHECK(result->get());
	CHECK(static_cast<const DummyTileImages&>(**result).key == key);
}

TEST_CASE("Test background loading tile cancels load when cancelled")
{
	auto imagesLoader = std::make_shared<DummyTileImagesLoader>();

	px_sched::Scheduler scheduler;
	scheduler.init();

	auto progressCallback = std::make_shared<TileProgressCallback>();
	QuadTreeTileKey key(0, 0, 0);
	auto result = std::make_shared<TileImagesPtr>();

	ConcurrentAsyncTileLoader loader(imagesLoader, &scheduler);
	loader.load(key, result, progressCallback);

	progressCallback->requestCancel();

	imagesLoader->doLoad = true;
	loader.waitForLoads();
	loader.update();

	CHECK(progressCallback->state == TileProgressCallback::State::FailedOrCanceled);
	CHECK(!result->get());
}
