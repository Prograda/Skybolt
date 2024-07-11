/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/Math/QuadTree.h>

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
	virtual ~AsyncTileLoader() = default;

	typedef std::shared_ptr<TileImagesPtr> TileImagesPtrPtr;

	virtual void load(const skybolt::QuadTreeTileKey& key, const TileImagesPtrPtr& result, const ProgressCallbackPtr& progress) = 0;

	virtual void waitForLoads() = 0;

	virtual void update() = 0;
};

} // namespace vis
} // namespace skybolt
