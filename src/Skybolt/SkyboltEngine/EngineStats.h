/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include <stdlib.h> //size_t

namespace skybolt {

struct EngineStats
{
	size_t terrainTileLoadQueueSize = 0;
	size_t featureTileLoadQueueSize = 0;
};

} // namespace skybolt