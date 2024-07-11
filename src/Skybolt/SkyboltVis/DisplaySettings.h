/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nlohmann/json.hpp>

namespace skybolt {
namespace vis {

//! Default constructed with reasonable defaults
struct DisplaySettings
{
	int multiSampleCount = 4; //!< Number of samples for MSAA
	int texturePoolSizeBytes =  512 * 1024 * 1024;
	bool vsync = true;
};

} // namespace vis
} // namespace skybolt