/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

std::unique_ptr<vis::VisRoot> createVisRoot(const EngineRoot& engineRoot);

void enableQuitOnEscapePress(const vis::VisRoot& visRoot);

} // namespace skybolt