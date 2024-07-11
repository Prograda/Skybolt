/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <vector>

namespace skybolt {
namespace vis {

class BrightStarCatelog
{
public:
	struct Star
	{
		float azimuth;
		float elevation;
		float magnitude;
	};

	static std::vector<Star> stars;

};

} // namespace vis
} // namespace skybolt