/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AsyncTileLoader.h"
#include "OsgTileFactory.h"
#include "TileImageLoader.h"

using namespace skybolt;

namespace skybolt {
namespace vis {

AsyncTileLoader::AsyncTileLoader(const TileImageLoaderPtr& tileImageLoader, const OsgTileFactoryPtr& tileFactory, px_sched::Scheduler* scheduler) :
	mTileImageLoader(tileImageLoader), mTileFactory(tileFactory), mScheduler(scheduler)
{
}

AsyncTileLoader::~AsyncTileLoader()
{
	for (const Request& request : mRequests)
	{
		request.imagesProcessCallback->cancel();
	}
	waitForLoads();
}

void AsyncTileLoader::load(const QuadTreeTileKey& key, Box2d latLonBounds, const OsgTilePtrPtr& result, const ProgressCallbackPtr& progress)
{
	Request request;
	request.key = key;
	request.latLonBounds = latLonBounds;
	request.tileResult = result;
	request.tileProgressCallback = progress;
	request.imagesResult.reset(new LeafTileDataPtr);
	request.imagesProcessCallback.reset(new TileProgressCallback);

	request.tileProgressCallback->state = TileProgressCallback::State::Loading;

	typedef std::shared_ptr<TileProgressCallback> TileProgressCallbackPtr;
	TileProgressCallbackPtr imagesProcessCallback = request.imagesProcessCallback;

	mRequests.push_back(request);

	mScheduler->run([=]() {
		imagesProcessCallback->state = TileProgressCallback::State::Loading;
		*request.imagesResult = mTileImageLoader->load(key, [=] {return imagesProcessCallback->isCanceled(); });
		imagesProcessCallback->state = *request.imagesResult ? TileProgressCallback::State::Loaded : TileProgressCallback::State::NotLoaded;
	}, &mLoadingTaskSync);
}

void AsyncTileLoader::waitForLoads()
{
	mScheduler->waitFor(mLoadingTaskSync);
}

void AsyncTileLoader::update()
{
	// Update requests
	static const int maxTileLoadsPerUpdate = 16; // tweek to give best performance. Smaller numbers give less frame stutters but potentially longer load delays.
	int tileLoads = 0;

	for (int i = 0; i < (int)mRequests.size(); ++i)
	{
		const Request& request = mRequests[i];
		if (request.tileProgressCallback->isCanceled())
		{
			request.imagesProcessCallback->cancel();
			request.tileProgressCallback->state = TileProgressCallback::State::NotLoaded;
			mRequests.erase(mRequests.begin() + i);
			--i;
		}
		else if (request.imagesProcessCallback->state == TileProgressCallback::State::Loading)
		{
			request.tileProgressCallback->state = TileProgressCallback::State::Loading;
		}
		else if (request.imagesResult->get() && tileLoads < maxTileLoadsPerUpdate)
		{
			const LeafTileData& data = *request.imagesResult->get();
			OsgTilePtr tile(new OsgTile(mTileFactory->createOsgTile(request.key, request.latLonBounds, data.heightMapImage, data.landMaskImage, data.albedoMapImage, data.attributeMapImage)));
			*request.tileResult = tile;

			request.tileProgressCallback->state = request.imagesProcessCallback->state.load();
			mRequests.erase(mRequests.begin() + i);

			++tileLoads;
			--i;
		}
	}
}

} // namespace vis
} // namespace skybolt
