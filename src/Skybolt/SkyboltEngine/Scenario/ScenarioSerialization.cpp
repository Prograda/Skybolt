/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioSerialization.h"
#include "Scenario.h"
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltEngine/Components/TemplateNameComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/Serialization/Serialization.h>
#include <SkyboltSim/World.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/StringVector.h>

using namespace skybolt::sim;

namespace skybolt {

static TimelineMode readTimelineMode(const std::string& str)
{
	if (str == "live")
	{
		return TimelineMode::Live;
	}
	else if (str == "free")
	{
		return TimelineMode::Free;
	}
	throw std::runtime_error("Invalid timeline mode: " + str);
}

static std::string toString(TimelineMode mode)
{
	switch (mode)
	{
	case TimelineMode::Live: return "live";
	case TimelineMode::Free: return "free";
	}
	return "live";
}

void readScenario(refl::TypeRegistry& typeRegistry, Scenario& scenario, const EntityFactoryFn& entityFactory, const nlohmann::json& json, EntityPersistenceFlags entityPersistenceFlags)
{
	SecondsD startTime = readOptionalOrDefault(json, "startTime", SecondsD(0));

	scenario.startJulianDate = json.at("julianDate");
	scenario.timeSource.setRange(TimeRange(startTime, startTime + json.at("duration").get<double>()));
	scenario.timeSource.setTime(readOptionalOrDefault(json, "currentTime", SecondsD(0)));
	scenario.timelineMode = readTimelineMode(readOptionalOrDefault<std::string>(json, "timelineMode", "live"));

	ifChildExists(json, "entities", [&] (const nlohmann::json& child) {
		readEntities(typeRegistry, scenario.world, entityFactory, child, entityPersistenceFlags);
	});
}

nlohmann::json writeScenario(refl::TypeRegistry& typeRegistry, const Scenario& scenario)
{
	const auto& timeRange = scenario.timeSource.getRange();
	nlohmann::json json;
	json["julianDate"] = scenario.startJulianDate;
	json["startTime"] = timeRange.start;
	json["duration"] = timeRange.end - timeRange.start;
	json["currentTime"] = scenario.timeSource.getTime();
	json["timelineMode"] = toString(scenario.timelineMode.get());
	json["entities"] = writeEntities(typeRegistry, scenario.world);
	return json;
}

static void readEntityComponents(refl::TypeRegistry& registry, World& world, sim::Entity& entity, const nlohmann::json& json)
{
	for (const auto& component : entity.getComponents())
	{
		refl::TypePtr type = registry.getOrCreateMostDerivedType(*component);
		ifChildExists(json, type->getName(), [&] (const nlohmann::json& componentJson) {
			refl::Instance instance = refl::makeRefInstance(registry, component.get());
			readReflectedObject(registry, instance, componentJson);
		});
	}

	// FIXME: we should remove all dynamically created components that already exist on entity,
	// however currently we can't distinguish between pre-existing components that were added
	// when the entity was first created (these should be kept) vs pre-existing components
	// that were dynamically created during the simulation (these should be removed).
}

static nlohmann::json writeEntityComponents(refl::TypeRegistry& registry, const sim::Entity& entity)
{
	nlohmann::json json;
	for (const auto& component : entity.getComponents())
	{
		refl::TypePtr type = registry.getOrCreateMostDerivedType(*component);
		refl::Instance instance = refl::makeRefInstance(registry, component.get());
		if (nlohmann::json componentJson = writeReflectedObject(registry, instance); !componentJson.is_null())
		{
			json[type->getName()] = componentJson;
		}
	}

	return json;
}

//! Creates entity if it does not already exist in the world, then updates with read state
static sim::EntityPtr readEntity(World& world, const EntityFactoryFn& factory, const std::string& name, const nlohmann::json& json)
{
	sim::EntityPtr entity = world.findObjectByName(name);
	if (!entity)
	{
		std::string templateName = json.at("template");
		entity = factory(templateName, name);
		world.addEntity(entity);
	}

	entity->setDynamicsEnabled(readOptionalOrDefault(json, "dynamicsEnabled", true));
	return entity;
}

static nlohmann::json writeEntity(refl::TypeRegistry& registry, const Entity& entity, const std::string& templateName)
{
	nlohmann::json json;
	json["template"] = templateName;
	json["dynamicsEnabled"] = entity.isDynamicsEnabled();
	json["components"] = writeEntityComponents(registry, entity);

	return json;
}

template <typename T>
inline std::set<T> toSet(const std::vector<T>& v)
{
	return std::set<T>(v.begin(), v.end());
}

static bool isSerializable(const Entity& entity)
{
	if (auto metadata = entity.getFirstComponent<ScenarioMetadataComponent>(); metadata)
	{
		return metadata->serializable;
	}
	return true; // Treat all entities as serializable by default.
}

//! @returns whether an entity should continue to exist even if it doesn't exist in in simulation state being load in.
static bool shouldPersistAcrossLoad(const Entity& entity, EntityPersistenceFlags entityPersistanceFlags)
{
	if (auto metadata = entity.getFirstComponent<ScenarioMetadataComponent>(); metadata)
	{
		if (entityPersistanceFlags.persistNonSerializable && !metadata->serializable) { return true; }
		if (entityPersistanceFlags.persistUserManaged && metadata->lifetimePolicy == ScenarioMetadataComponent::LifetimePolicy::User)  { return true; }
	}
	return false; // Entities should not persist by default.
}

void readEntities(refl::TypeRegistry& registry, World& world, const EntityFactoryFn& factory, const nlohmann::json& json, EntityPersistenceFlags entityPersistenceFlags)
{
	// Make a set of names of entities in the world that should be removed if they don't exist in the serialized state being read.
	std::set<std::string> oldEntityNames;
	for (const auto& entity : world.getEntities())
	{
		if (shouldPersistAcrossLoad(*entity, entityPersistenceFlags)) { continue; }

		if (const std::string& name = getName(*entity); !name.empty())
		{
			oldEntityNames.insert(name);
		}
	}

	// Read entities into world
	std::vector<sim::EntityPtr> entities;

	for (const auto& [key, entityJson] : json.items())
	{
		EntityPtr entity = readEntity(world, factory, key, entityJson);
		entities.push_back(entity);
		oldEntityNames.erase(getName(*entity));
	}

	// Remove old entities from world
	for (const auto& entityName : oldEntityNames)
	{
		if (Entity* entity = world.findObjectByName(entityName).get(); entity)
		{
			world.removeEntity(entity);
		}
	}

	// Read components after all entities exist, in case a component refers to an entity
	int i = 0;
	for (const auto& [key, entity] : json.items())
	{
		ifChildExists(entity, "components", [&](const nlohmann::json& components){
			readEntityComponents(registry, world, *entities[i], components);
		});
		++i;
	}
}

nlohmann::json writeEntities(refl::TypeRegistry& registry, const World& world)
{
	nlohmann::json json;
	for (const EntityPtr& entity : world.getEntities())
	{
		if (isSerializable(*entity))
		{
			const std::string& name = getName(*entity);
			auto templateNameComponent = entity->getFirstComponent<TemplateNameComponent>();
			if (!name.empty() && templateNameComponent)
			{
				json[name] = writeEntity(registry, *entity, templateNameComponent->name);
			}
		}
	}

	return json;
}

} // namespace skybolt