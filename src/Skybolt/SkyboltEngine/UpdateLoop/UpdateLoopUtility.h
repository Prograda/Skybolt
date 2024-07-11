/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "UpdateLoop.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

using SimPausedPredicate = std::function<bool()>;

void runMainLoop(vis::VisRoot& visRoot, EngineRoot& engineRoot, UpdateLoop::ShouldExit shouldExit, SimPausedPredicate paused = [] {return false; });

} // namespace skybolt