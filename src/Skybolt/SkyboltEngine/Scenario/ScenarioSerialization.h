/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <nlohmann/json.hpp>

namespace skybolt {

void loadScenario(Scenario& scenario, const nlohmann::json& value);

nlohmann::json saveScenario(const Scenario& scenario);

void loadEntities(sim::World& world, EntityFactory& factory, const nlohmann::json& value);

nlohmann::json saveEntities(const sim::World& world);

} // namespace skybolt