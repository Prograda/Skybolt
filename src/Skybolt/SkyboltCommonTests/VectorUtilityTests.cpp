/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltCommon/VectorUtility.h>

using namespace skybolt;

TEST_CASE("std::vector findFirst")
{
	std::vector<int> v = {1, 2};
	CHECK(findFirst(v, 1) == &v[0]);
	CHECK(findFirst(v, 3) == nullptr);
}

TEST_CASE("std::vector contains")
{
	std::vector<int> v = {1, 2};
	CHECK(contains(v, 1));
	CHECK(!contains(v, 3));
}

TEST_CASE("std::vector eraseFirst")
{
	std::vector<int> v = {2, 1, 1};
	CHECK(eraseFirst(v, 1));
	CHECK(v == std::vector<int>({2, 1}));

	CHECK(!eraseFirst(v, 3));
	CHECK(v == std::vector<int>({2, 1}));
}

TEST_CASE("std::vector fastUnstableErase")
{
	std::vector<int> v = {1, 2, 3};
	fastUnstableEraseAtIndex(v, 0);
	CHECK(v == std::vector<int>({3, 2}));

	fastUnstableEraseAtIndex(v, 2);
	CHECK(v == std::vector<int>({3}));

	fastUnstableEraseAtIndex(v, 0);
	CHECK(v.empty());
}
