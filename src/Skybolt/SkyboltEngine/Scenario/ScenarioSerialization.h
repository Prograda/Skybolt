/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltReflect/SkyboltReflectFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <nlohmann/json.hpp>

namespace skybolt {

using EntityFactoryFn = std::function<sim::EntityPtr(const std::string& templateName, const std::string& instanceName)>;

struct EntityPersistenceFlags
{
	bool persistNonSerializable = false;
	bool persistUserManaged = false;
};

//! Reads scenario state from json. Any existing world state will be overwritten.
void readScenario(refl::TypeRegistry& typeRegistry, Scenario& scenario, const EntityFactoryFn& entityFactory, const nlohmann::json& value, EntityPersistenceFlags entityPersistenceFlags = {});

nlohmann::json writeScenario(refl::TypeRegistry& typeRegistry, const Scenario& scenario);

void readEntities(refl::TypeRegistry& registry, sim::World& world, const EntityFactoryFn& factory, const nlohmann::json& value, EntityPersistenceFlags entityPersistenceFlags = {});

nlohmann::json writeEntities(refl::TypeRegistry& registry, const sim::World& world);

} // namespace skybolt