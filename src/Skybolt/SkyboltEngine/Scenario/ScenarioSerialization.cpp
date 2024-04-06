#include "ScenarioSerialization.h"
#include "Scenario.h"
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/Serialization/Serialization.h>
#include <SkyboltSim/World.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/StringVector.h>

using namespace skybolt::sim;

namespace skybolt {

void readScenario(refl::TypeRegistry& typeRegistry, Scenario& scenario, EntityFactory& entityFactory, const nlohmann::json& json)
{
	scenario.startJulianDate = json.at("julianDate");
	scenario.timeSource.setRange(TimeRange(0, json.at("duration")));

	ifChildExists(json, "entities", [&] (const nlohmann::json& child) {
		readEntities(typeRegistry, scenario.world, entityFactory, child);
	});
}

nlohmann::json writeScenario(refl::TypeRegistry& typeRegistry, const Scenario& scenario)
{
	nlohmann::json json;
	json["julianDate"] = scenario.startJulianDate;
	json["duration"] = scenario.timeSource.getRange().end;
	json["entities"] = writeEntities(typeRegistry, scenario.world);
	return json;
}

static void readEntityComponents(refl::TypeRegistry& registry, World& world, sim::Entity& entity, const nlohmann::json& json)
{
	for (const auto& component : entity.getComponents())
	{
		refl::TypePtr type = registry.getOrCreateMostDerivedType(*component);
		ifChildExists(json, type->getName(), [&] (const nlohmann::json& componentJson) {
			refl::Instance instance = refl::createNonOwningInstance(&registry, component.get());
			readReflectedObject(registry, instance, componentJson);
		});
	}
}

static nlohmann::json writeEntityComponents(refl::TypeRegistry& registry, const sim::Entity& entity)
{
	nlohmann::json json;
	for (const auto& component : entity.getComponents())
	{
		refl::TypePtr type = registry.getOrCreateMostDerivedType(*component);
		refl::Instance instance = refl::createNonOwningInstance(&registry, component.get());
		if (nlohmann::json componentJson = writeReflectedObject(registry, instance); !componentJson.is_null())
		{
			json[type->getName()] = componentJson;
		}
	}

	return json;
}

static sim::EntityPtr readEntity(World& world, EntityFactory& factory, const std::string& name, const nlohmann::json& json)
{
	std::string templateName = json.at("template");
	sim::EntityPtr body = factory.createEntity(templateName, name);
	world.addEntity(body);

	body->setDynamicsEnabled(readOptionalOrDefault(json, "dynamicsEnabled", true));

	return body;
}

static nlohmann::json writeEntity(refl::TypeRegistry& registry, const Entity& entity, const std::string& templateName)
{
	nlohmann::json json;
	json["template"] = templateName;
	json["dynamicsEnabled"] = entity.isDynamicsEnabled();
	json["components"] = writeEntityComponents(registry, entity);

	return json;
}

void readEntities(refl::TypeRegistry& registry, World& world, EntityFactory& factory, const nlohmann::json& json)
{
	std::vector<sim::EntityPtr> entities;

	for (const auto& [key, entity] : json.items())
	{
		entities.push_back(readEntity(world, factory, key, entity));
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

static bool isSerializable(const Entity& entity)
{
	if (auto metadata = entity.getFirstComponent<ScenarioMetadataComponent>(); metadata)
	{
		return metadata->serializable;
	}
	return true;
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