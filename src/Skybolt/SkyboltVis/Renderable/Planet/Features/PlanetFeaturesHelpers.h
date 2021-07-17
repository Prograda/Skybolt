/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <vector>

namespace skybolt {
namespace mapfeatures {

struct RoadJunction
{
	enum class StartOrEnd
	{
		Start,
		End
	};

	struct Item
	{
		std::shared_ptr<struct Road> road;
		StartOrEnd startOrEnd; // which end of the road the junction joins to
	};

	std::vector<Item> roads;
};

void joinRoadsAtJunction(const RoadJunction& junction);

} // namespace mapfeatures
} // namespace skybolt