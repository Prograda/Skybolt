/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ConcurrentAsyncTileLoader.h"
#include "TileImagesLoader.h"

using namespace skybolt;

namespace skybolt {
namespace vis {

ConcurrentAsyncTileLoader::ConcurrentAsyncTileLoader(const TileImagesLoaderPtr& tileImageLoader, px_sched::Scheduler* scheduler) :
	mTileImageLoader(tileImageLoader), mScheduler(scheduler)
{
}

ConcurrentAsyncTileLoader::~ConcurrentAsyncTileLoader()
{
	for (const Request& request : mRequests)
	{
		request.progressCallback->requestCancel();
	}
	waitForLoads();
}

void ConcurrentAsyncTileLoader::load(const QuadTreeTileKey& key, const TileImagesPtrPtr& result, const ProgressCallbackPtr& progress)
{
	Request request;
	request.key = key;
	request.result = result;
	request.progressCallback = progress;

	typedef std::shared_ptr<TileProgressCallback> TileProgressCallbackPtr;
	request.progressCallback->state = TileProgressCallback::State::Loading;

	mRequests.push_back(request);

	mScheduler->run([=]() {
		*request.result = mTileImageLoader->load(key, [=] {return progress->isCancelRequested(); });
		request.progressCallback->state = *request.result ? TileProgressCallback::State::Loaded : TileProgressCallback::State::FailedOrCanceled;
	}, &mLoadingTaskSync);
}

void ConcurrentAsyncTileLoader::waitForLoads()
{
	mScheduler->waitFor(mLoadingTaskSync);
}

void ConcurrentAsyncTileLoader::update()
{
	// Update requests
	static const int maxTileLoadsPerUpdate = 16; // tweek to give best performance. Smaller numbers give less frame stutters but potentially longer load delays.
	int tileLoads = 0;

	for (int i = 0; i < (int)mRequests.size(); ++i)
	{
		const Request& request = mRequests[i];
		auto state = request.progressCallback->state.load();
		if (state == TileProgressCallback::State::FailedOrCanceled)
		{
			mRequests.erase(mRequests.begin() + i);
			--i;
		}
		else if (request.result->get() && tileLoads < maxTileLoadsPerUpdate)
		{
			mRequests.erase(mRequests.begin() + i);

			++tileLoads;
			--i;
		}
	}
}

} // namespace vis
} // namespace skybolt
