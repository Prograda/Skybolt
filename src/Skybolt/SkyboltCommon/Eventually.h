/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <chrono>
#include <functional>
#include <thread>

namespace skybolt {

inline bool eventually(const std::function<bool()>& fn, int tryCount = 20, const std::chrono::milliseconds& retryDelay = std::chrono::milliseconds(10))
{
	for (int i = 0; i < tryCount; ++i)
	{
		if (fn())
		{
			return true;
		}
		std::this_thread::sleep_for(retryDelay);
	}
	return false;
}

} // namespace skybolt