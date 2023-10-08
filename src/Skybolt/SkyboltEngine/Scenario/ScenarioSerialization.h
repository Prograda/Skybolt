/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltReflection/SkyboltReflectionFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <nlohmann/json.hpp>

namespace skybolt {

void readScenario(refl::TypeRegistry& typeRegistry, Scenario& scenario, EntityFactory& entityFactory, const nlohmann::json& value);

nlohmann::json writeScenario(refl::TypeRegistry& typeRegistry, const Scenario& scenario);

void readEntities(refl::TypeRegistry& registry, sim::World& world, EntityFactory& factory, const nlohmann::json& value);

nlohmann::json writeEntities(refl::TypeRegistry& registry, const sim::World& world);

} // namespace skybolt