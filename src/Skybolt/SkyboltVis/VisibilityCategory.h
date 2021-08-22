/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace vis {

struct VisibilityCategory
{
	static constexpr int shadowCaster = 1;
	
	// Total number of above categories
	static constexpr int categoryCount = 1;
	
	static constexpr int defaultCategories = shadowCaster;
};

} // namespace vis
} // namespace skybolt
