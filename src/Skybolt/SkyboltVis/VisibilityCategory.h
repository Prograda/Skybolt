/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace vis {

struct VisibilityCategory
{
	static constexpr int primary = 1 << 0; //!< primary visibility, i.e the main camera render pass
	static constexpr int shadowCaster = 1 << 1; //!< visible to shadow casters
	
	// Total number of above categories. Provided for users to add custom categories after predefined ones.
	static constexpr int categoryCount = 2;
	
	static constexpr int defaultCategories = primary;
};

} // namespace vis
} // namespace skybolt
