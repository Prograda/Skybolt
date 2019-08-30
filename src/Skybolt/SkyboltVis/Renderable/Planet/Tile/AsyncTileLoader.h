/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OsgTile.h"
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
		NotLoaded,
		Loading,
		Loaded
	};

	std::atomic<State> state = State::NotLoaded;

	bool isCanceled() const { return canceled; }
	void cancel() { canceled = true; }

private:
	std::atomic<bool> canceled = false;
};

typedef std::shared_ptr<OsgTilePtr> OsgTilePtrPtr;

class AsyncTileLoader
{
public:
	AsyncTileLoader(const TileImageLoaderPtr& tileImageLoader, const OsgTileFactoryPtr& tileFactory, px_sched::Scheduler* scheduler);

	~AsyncTileLoader();

	void load(const skybolt::QuadTreeTileKey& key, Box2d latLonBounds, const OsgTilePtrPtr& result, const ProgressCallbackPtr& progress);

	void waitForLoads();

	void update();

private:
	TileImageLoaderPtr mTileImageLoader;
	OsgTileFactoryPtr mTileFactory;
	px_sched::Scheduler* mScheduler;
	px_sched::Sync mLoadingTaskSync;

	typedef std::shared_ptr<struct LeafTileData> LeafTileDataPtr;
	typedef std::shared_ptr<LeafTileDataPtr> LeafTileDataPtrPtr;

	struct Request
	{
		skybolt::QuadTreeTileKey key;
		Box2d latLonBounds;
		OsgTilePtrPtr tileResult; //!< The outer pointer is used to share the lifetime of the inner pointer between producer and consumer. The inner pointer is changed from nullptr to containing a valid object by producer.
		LeafTileDataPtrPtr imagesResult; //!< The outer pointer is used to share the lifetime of the inner pointer between producer and consumer. The inner pointer is changed from nullptr to containing a valid object by producer.
		ProgressCallbackPtr tileProgressCallback;
		ProgressCallbackPtr imagesProcessCallback;
	};

	std::vector<Request> mRequests;
};

} // namespace vis
} // namespace skybolt
