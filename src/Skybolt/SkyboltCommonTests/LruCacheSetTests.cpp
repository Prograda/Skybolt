/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/LruCacheSet.h>

using namespace skybolt;

constexpr int capacity = 5;

TEST_CASE("LruCacheSet put item")
{
	LruCacheSet<int> cache(capacity);
	
	int value = 5;
	CHECK(!cache.exists(value));

	cache.put(value);
	CHECK(cache.exists(value));
}

TEST_CASE("LruCacheSet least recently used item is pruned when cache capacity exceeded")
{
	LruCacheSet<int> cache(capacity);

	// Put items
	for (int i = 0; i < capacity; ++i)
	{
		int value = i;
		cache.put(value);
	}
	
	// Put the first item again
	cache.put(0);

	// The least recently used item is now the second item.
	// Check that the second item is removed when another item is added.
	cache.put(55);
	CHECK(cache.exists(55));
	CHECK(!cache.exists(1));
}
