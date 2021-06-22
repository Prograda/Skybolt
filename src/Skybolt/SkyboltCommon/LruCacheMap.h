/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <assert.h>
#include <list>
#include <unordered_map>

namespace skybolt {

//! Key-value map that removes least recently used items when capacity is full
template <typename KeyT, typename ValueT>
struct LruCacheMap
{
	LruCacheMap(size_t capacity) :
		mCapacity(capacity)
	{
	}

	//! @returns true on put
	bool putSafe(const KeyT& key, const ValueT& value)
	{
		auto it = mEntries.find(key);
		if (it == mEntries.end())
		{
			mQueue.push_front(std::make_pair(key, value));
			mEntries[key] = mQueue.begin();
			prune();
			return true;
		}

		return false;
	}

	void put(const KeyT& key, const ValueT& value)
	{
		assert(mEntries.find(key) == mEntries.end());

		mQueue.push_front(std::make_pair(key, value));
		mEntries[key] = mQueue.begin();
		prune();
	}

	bool get(const KeyT& key, ValueT& valueOut)
	{
		auto it = mEntries.find(key);
		if (it != mEntries.end())
		{
			mQueue.splice(mQueue.begin(), mQueue, it->second); // move item to the beginning of the queue
			valueOut = it->second->second;
			return true;
		}
		return false;
	}

	size_t size() const
	{
		return mEntries.size();
	}

	//! Tests whether item exists without 'using' the item (i.e caching is unaffected)
	bool exists(const KeyT& key) const
	{
		return mEntries.find(key) != mEntries.end();
	}

private:
	void prune()
	{
		while (mEntries.size() > mCapacity)
		{
			auto it = mQueue.end();
			it--;
			mEntries.erase(it->first);
			mQueue.pop_back();
		}
	}

private:
	size_t mCapacity;

	std::list<std::pair<KeyT, ValueT>> mQueue;
	std::unordered_map<KeyT, decltype(mQueue.begin())> mEntries;
};

} // namespace skybolt