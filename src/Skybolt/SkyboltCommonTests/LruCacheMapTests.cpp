/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/LruCacheMap.h>

using namespace skybolt;

TEST_CASE("LruCacheMap put and get item")
{
	int capacity = 5;
	LruCacheMap<std::string, int> cache(capacity);
	
	int value = 5;
	CHECK(cache.putSafe("test", value));
	CHECK(cache.exists("test"));

	CHECK(!cache.putSafe("test", value));

	int result;
	CHECK(cache.get("test", result));
	CHECK(result == value);
}

TEST_CASE("LruCacheMap least recently used item is pruned when cache capacity exceeded")
{
	int capacity = 5;
	LruCacheMap<std::string, int> cache(capacity);

	// Put items
	for (int i = 0; i < capacity; ++i)
	{
		cache.put(std::to_string(i), i);
	}
	// The cache order should be [4, 3, 2, 1, 0]
	
	// Get the first item
	int value;
	CHECK(cache.get("0", value));
	CHECK(value == 0);
	CHECK(cache.exists("0"));

	// The cache order should now be [0, 4, 3, 2, 1]

	// Check that 1 is removed when another item is added.
	value = 55;
	CHECK(cache.exists("1"));
	cache.put("newItem", value);

	// The cache order should now be [55, 0, 4, 3, 2]

	CHECK(!cache.exists("1"));

	// Get 3 with a get()
	CHECK(cache.get("3", value));

	// The cache order should now be [3, 55, 0, 4, 2]

	// Check that 2 is removed when another item is added.
	value = 88;
	CHECK(cache.exists("2"));
	cache.put("anotherNewItem", value);

	// The cache order should now be [88, 55, 0, 4, 3]

	CHECK(!cache.exists("2"));
}
