/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <list>
#include <unordered_map>

namespace skybolt {

//! Set that removes least recently used items when capacity is full
template <typename ValueT>
struct LruCacheSet
{
	LruCacheSet(size_t capacity) :
		mCapacity(capacity)
	{
	}

	void put(const ValueT& value)
	{
		auto it = mEntries.find(value);
		if (it == mEntries.end())
		{
			// Add new item
			mQueue.push_front(value);
			mEntries[value] = mQueue.begin();
			prune();
		}
		else
		{
			// Move item to the beginning of the queue
			mQueue.splice(mQueue.begin(), mQueue, it->second);
		}
	}

	size_t size() const
	{
		return mEntries.size();
	}

	bool exists(const ValueT& value) const
	{
		return mEntries.find(value) != mEntries.end();
	}

private:
	void prune()
	{
		while (mEntries.size() > mCapacity)
		{
			auto it = mQueue.end();
			it--;
			mEntries.erase(*it);
			mQueue.pop_back();
		}
	}

private:
	size_t mCapacity;

	std::list<ValueT> mQueue;
	std::unordered_map<ValueT, decltype(mQueue.begin())> mEntries;
};

} // namespace skybolt