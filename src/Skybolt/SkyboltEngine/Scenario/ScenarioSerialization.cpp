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

void readScenario(Scenario& scenario, const nlohmann::json& object)
{
	scenario.startJulianDate = object.at("julianDate");
	scenario.timeSource.setRange(TimeRange(0, object.at("duration")));
}

nlohmann::json writeScenario(const Scenario& scenario)
{
	nlohmann::json json;
	json["julianDate"] = scenario.startJulianDate;
	json["duration"] = scenario.timeSource.getRange().end;
	return json;
}

static void readEntityComponents(World& world, sim::Entity& entity, const nlohmann::json& json)
{
	for (const auto& component : entity.getComponents())
	{
		std::string typeName = sim::getType(*component).get_name().to_string();
		ifChildExists(json, typeName, [&] (const nlohmann::json& componentJson) {
			readReflectedObject(rttr::instance(*component), componentJson);
		});
	}
}

static nlohmann::json writeEntityComponents(const sim::Entity& entity)
{
	nlohmann::json json;
	for (const auto& component : entity.getComponents())
	{
		const rttr::type& type = sim::getType(*component);
		std::string typeName = type.get_name().to_string();
		if (nlohmann::json componentJson = writeReflectedObject(*component); !componentJson.is_null())
		{
			json[typeName] = componentJson;
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

static nlohmann::json writeEntity(const Entity& entity, const std::string& templateName)
{
	nlohmann::json json;
	json["template"] = templateName;
	json["dynamicsEnabled"] = entity.isDynamicsEnabled();
	json["components"] = writeEntityComponents(entity);

	return json;
}

void readEntities(World& world, EntityFactory& factory, const nlohmann::json& json)
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
			readEntityComponents(world, *entities[i], components);
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

nlohmann::json writeEntities(const World& world)
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
				json[name] = writeEntity(*entity, templateNameComponent->name);
			}
		}
	}

	return json;
}

} // namespace skybolt