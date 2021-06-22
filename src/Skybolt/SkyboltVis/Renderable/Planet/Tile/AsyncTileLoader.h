/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/OsgBox2.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <px_sched/px_sched.h>
#include <atomic>

namespace skybolt {
namespace vis {

struct TileProgressCallback
{
	enum class State
	{
		Loading,
		Loaded,
		FailedOrCanceled
	};

	std::atomic<State> state = State::Loading;

	bool isCancelRequested() const { return canceledRequested; }
	void requestCancel() { canceledRequested = true; }

private:
	std::atomic<bool> canceledRequested = false;
};

class AsyncTileLoader
{
public:
	AsyncTileLoader(const TileImagesLoaderPtr& tileImageLoader, px_sched::Scheduler* scheduler);

	~AsyncTileLoader();

	typedef std::shared_ptr<TileImagesPtr> TileImagesPtrPtr;

	void load(const skybolt::QuadTreeTileKey& key, const TileImagesPtrPtr& result, const ProgressCallbackPtr& progress);

	void waitForLoads();

	void update();

private:
	TileImagesLoaderPtr mTileImageLoader;
	px_sched::Scheduler* mScheduler;
	px_sched::Sync mLoadingTaskSync;

	struct Request
	{
		skybolt::QuadTreeTileKey key;
		TileImagesPtrPtr result; //!< The outer pointer is used to share the lifetime of the inner pointer between producer and consumer. The inner pointer is changed from nullptr to containing a valid object by producer.
		ProgressCallbackPtr progressCallback;
	};

	std::vector<Request> mRequests;
};

} // namespace vis
} // namespace skybolt
