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

#include "ScenarioSerialization.h"
#include "Scenario.h"
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltEngine/VisObjectsComponent.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/CameraController/LatLonSettable.h>
#include <SkyboltSim/CameraController/Pitchable.h>
#include <SkyboltSim/CameraController/Yawable.h>
#include <SkyboltSim/CameraController/Zoomable.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Renderable/Planet/Planet.h>
#include <SkyboltVis/Renderable/Water/WaterMaterial.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/StringVector.h>

using namespace skybolt::sim;

namespace skybolt {

void loadScenario(Scenario& scenario, const nlohmann::json& object)
{
	scenario.startJulianDate = object.at("julianDate");
	scenario.timeSource.setRange(TimeRange(0, object.at("duration")));
}

nlohmann::json saveScenario(const Scenario& scenario)
{
	nlohmann::json json;
	json["julianDate"] = scenario.startJulianDate;
	json["duration"] = scenario.timeSource.getRange().end;
	return json;
}

static skybolt::StringVector loadEntityAttachments(const nlohmann::json& json)
{
	skybolt::StringVector result;
	for (const nlohmann::json& value : json)
	{
		result.push_back(value);
	}
	return result;
}

typedef std::map<std::string, sim::CameraControllerPtr> CameraModes;

static void loadCameraController(CameraControllerSelector& cameraControllerSelector, const World& world, const nlohmann::json& j)
{
	std::string name = j.at("mode");
	cameraControllerSelector.selectController(name);

	if (j.contains("target"))
	{
		EntityPtr target = findObjectByName(world, j.at("target"));
		if (target)
		{
			cameraControllerSelector.setTarget(target.get());
		}
	}

	if (j.contains("modes"))
	{
		nlohmann::json modes = j.at("modes");
		for (const auto& item : cameraControllerSelector.getControllers())
		{
			std::string name = item.first;
			if (modes.contains(name))
			{
				nlohmann::json mode = modes.at(name);
				CameraController* controller = item.second.get();
				if (auto latLonSettable = dynamic_cast<LatLonSettable*>(controller))
				{
					latLonSettable->setLatLon(sim::LatLon(mode.at("lat"), mode.at("lon")));
				}
				if (auto pitchable = dynamic_cast<Pitchable*>(controller))
				{
					pitchable->setPitch(mode.at("pitch"));
				}
				if (auto yawable = dynamic_cast<Yawable*>(controller))
				{
					yawable->setYaw(mode.at("yaw"));
				}
				if (auto zoomable = dynamic_cast<Zoomable*>(controller))
				{
					zoomable->setZoom(mode.at("zoom"));
				}
			}
		}
	}
}

static Entity* findObjectInWorld(const World& world, const std::string& name)
{
	for (const EntityPtr& entity : world.getEntities())
	{
		if (getName(*entity) == name)
			return entity.get();
	}
	return nullptr;
}

static sim::AttachmentComponent* findFreeAttachmentAcceptingEntityTemplate(const Entity& entity, const std::string& templateName)
{
	for (auto component : entity.getComponentsOfType<sim::AttachmentComponent>())
	{
		if (!component->getTarget() && component->getEntityTemplate() == templateName)
		{
			return component.get();
		}
	}
	return nullptr;
}

static void findAndAttachEntityByName(const Entity& parent, const World& world, const std::string& name)
{
	Entity* target = findObjectInWorld(world, name);
	if (target)
	{
		if (auto templateNameComponent = target->getFirstComponent<TemplateNameComponent>())
		{
			if (AttachmentComponent* attachment = findFreeAttachmentAcceptingEntityTemplate(parent, templateNameComponent->name))
			{
				attachment->resetTarget(target);
			}
		}
	}
}

static sim::EntityPtr loadEntity(World& world, EntityFactory& factory, const std::string& name, const nlohmann::json& json)
{
	std::string templateName = json.at("template");
	sim::EntityPtr body = factory.createEntity(templateName, name);
	world.addEntity(body);

	ifChildExists(json, "position", [=](const nlohmann::json& object) {
		setPosition(*body, readVector3(object));
	});

	ifChildExists(json, "orientation", [=](const nlohmann::json& object) {
		setOrientation(*body, readQuaternion(object));
	});

	ifChildExists(json, "velocity", [=](const nlohmann::json& object) {
		setVelocity(*body, readVector3(object));
	});

	body->setDynamicsEnabled(readOptionalOrDefault(json, "dynamicsEnabled", true));

	return body;
}

static void loadEntityComponents(World& world, sim::Entity& entity, const nlohmann::json& json)
{
	if (auto cameraControllerComponent = entity.getFirstComponent<CameraControllerComponent>())
	{
		if (auto i = json.find("cameraController"); i != json.end())
		{
			if (auto selector = dynamic_cast<CameraControllerSelector*>(cameraControllerComponent->cameraController.get()))
			{
				loadCameraController(*selector, world, i.value());
			}
		}
	}

	if (auto i = json.find("attachments"); i != json.end())
	{
		skybolt::StringVector attachments = loadEntityAttachments(i.value());
		for (const std::string& name : attachments)
		{
			findAndAttachEntityByName(entity, world, name);
		}
	}

	if (vis::Planet* planet = getFirstVisObject<vis::Planet>(entity).get(); planet)
	{
		ifChildExists(json, "planet", [&] (const nlohmann::json& j) {
			if (const auto& material = planet->getWaterMaterial(); material)
			{
				ifChildExists(j, "waveHeight", [&]  (const nlohmann::json& j) {
					material->setWaveHeight(j.get<double>());
				});
			}
		});
	}
}

void loadEntities(World& world, EntityFactory& factory, const nlohmann::json& json)
{
	std::vector<sim::EntityPtr> entities;

	for (const auto& [key, entity] : json.items())
	{
		entities.push_back(loadEntity(world, factory, key, entity));
	}

	// Load components after all entities exist, in case a component refers to an entity
	int i = 0;
	for (const auto& [key, entity] : json.items())
	{
		loadEntityComponents(world, *entities[i], entity);
		++i;
	}
}

static nlohmann::json saveEntityAttachments(const Entity& entity)
{
	nlohmann::json json;
	for (auto attachment : entity.getComponentsOfType<AttachmentComponent>())
	{
		if (Entity* target = attachment->getTarget())
		{
			json.push_back(getName(*target));
		}
	}
	return json;
}

static std::string getName(const sim::CameraController& controller, const CameraModes& mCameraModes)
{
	for (const auto& item : mCameraModes)
	{
		if (item.second.get() == &controller)
		{
			return item.first;
		}
	}
	return "";
}

static nlohmann::json saveCameraController(CameraControllerSelector& cameraControllerSelector)
{
	nlohmann::json j;
	j["mode"] = cameraControllerSelector.getSelectedControllerName();

	if (cameraControllerSelector.getTarget())
	{
		j["target"] = getName(*cameraControllerSelector.getTarget());
	}

	nlohmann::json modes;
	for (const auto& item : cameraControllerSelector.getControllers())
	{
		std::string name = item.first;
		nlohmann::json mode;

		CameraController* controller = item.second.get();
		if (auto latLonSettable = dynamic_cast<LatLonSettable*>(controller))
		{
			sim::LatLon latLon = latLonSettable->getLatLon();
			mode["lat"] = latLon.lat;
			mode["lon"] = latLon.lon;
		}
		if (auto pitchable = dynamic_cast<Pitchable*>(controller))
		{
			mode["pitch"] = pitchable->getPitch();
		}
		if (auto yawable = dynamic_cast<Yawable*>(controller))
		{
			mode["yaw"] = yawable->getYaw();
		}
		if (auto zoomable = dynamic_cast<Zoomable*>(controller))
		{
			mode["zoom"] = zoomable->getZoom();
		}

		modes[name] = mode;
	}
	j["modes"] = modes;

	return j;
}

static nlohmann::json saveEntity(const Entity& entity, const std::string& templateName)
{
	nlohmann::json json;
	json["template"] = templateName;
	json["dynamicsEnabled"] = entity.isDynamicsEnabled();
	writeIfNotEmpty(json, "attachments", saveEntityAttachments(entity));

	auto position = getPosition(entity);
	if (position)
	{
		json["position"] = writeJson(*position);
	}

	auto velocity = getVelocity(entity);
	if (velocity)
	{
		json["velocity"] = writeJson(*velocity);
	}

	auto orientation = getOrientation(entity);
	if (orientation)
	{
		json["orientation"] = writeJson(*orientation);
	}

	auto cameraController = entity.getFirstComponent<CameraControllerComponent>();
	if (cameraController)
	{
		auto selector = dynamic_cast<CameraControllerSelector*>(cameraController->cameraController.get());
		if (selector)
		{
			json["cameraController"] = saveCameraController(*selector);
		}
	}

	if (vis::Planet* planet = getFirstVisObject<vis::Planet>(entity).get(); planet)
	{
		if (const auto& material = planet->getWaterMaterial(); material)
		{
			nlohmann::json planetJson;
			planetJson["waveHeight"] = material->getWaveHeight();
			json["planet"] = planetJson;
		}
	}

	return json;
}

nlohmann::json saveEntities(const World& world)
{
	nlohmann::json json;
	for (const EntityPtr& entity : world.getEntities())
	{
		if (!entity->getFirstComponent<ProceduralLifetimeComponent>())
		{
			const std::string& name = getName(*entity);
			auto templateNameComponent = entity->getFirstComponent<TemplateNameComponent>();
			if (!name.empty() && templateNameComponent)
			{
				json[name] = saveEntity(*entity, templateNameComponent->name);
			}
		}
	}
	return json;
}

} // namespace skybolt