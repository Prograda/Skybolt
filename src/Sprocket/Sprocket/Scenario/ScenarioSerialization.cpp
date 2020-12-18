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
#include <Sprocket/JsonHelpers.h>
#include <SkyboltEngine/Scenario.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/CameraController/LatLonSettable.h>
#include <SkyboltSim/CameraController/Pitchable.h>
#include <SkyboltSim/CameraController/Yawable.h>
#include <SkyboltSim/CameraController/Zoomable.h>
#include <SkyboltSim/World.h>
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/StringVector.h>

#include <QJsonArray>
#include <QStringList>

using namespace skybolt;
using namespace skybolt::sim;

void loadScenario(Scenario& scenario, const QJsonObject& object)
{
	scenario.startJulianDate = object["julianDate"].toDouble();
	scenario.timeSource.setRange(TimeRange(0, object["duration"].toDouble()));
}

QJsonObject saveScenario(const Scenario& scenario)
{
	QJsonObject json;
	json["julianDate"] = scenario.startJulianDate;
	json["duration"] = scenario.timeSource.getRange().end;
	return json;
}

static skybolt::StringVector loadEntityAttachments(const QJsonArray& json)
{
	skybolt::StringVector result;
	for (const QJsonValue& value : json)
	{
		result.push_back(value.toString().toStdString());
	}
	return result;
}

typedef std::map<std::string, sim::CameraControllerPtr> CameraModes;

static void loadCameraController(CameraControllerSelector& cameraControllerSelector, const World& world, const QJsonObject& j)
{
	std::string name = j["mode"].toString().toStdString();
	cameraControllerSelector.selectController(name);

	if (j.contains("target"))
	{
		EntityPtr target = findObjectByName(world, j["target"].toString().toStdString());
		if (target)
		{
			cameraControllerSelector.setTarget(target.get());
		}
	}

	if (j.contains("modes"))
	{
		QJsonObject modes = j["modes"].toObject();
		for (const auto& item : cameraControllerSelector.getControllers())
		{
			QString name = QString::fromStdString(item.first);
			if (modes.contains(name))
			{
				QJsonObject mode = modes[name].toObject();
				CameraController* controller = item.second.get();
				if (auto latLonSettable = dynamic_cast<LatLonSettable*>(controller))
				{
					latLonSettable->setLatLon(sim::LatLon(mode["lat"].toDouble(), mode["lon"].toDouble()));
				}
				if (auto pitchable = dynamic_cast<Pitchable*>(controller))
				{
					pitchable->setPitch(mode["pitch"].toDouble());
				}
				if (auto yawable = dynamic_cast<Yawable*>(controller))
				{
					yawable->setYaw(mode["yaw"].toDouble());
				}
				if (auto zoomable = dynamic_cast<Zoomable*>(controller))
				{
					zoomable->setZoom(mode["zoom"].toDouble());
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

static void findAndAttachEntityByName(Entity& parent, const World& world, const std::string& name)
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

static sim::EntityPtr loadEntity(World& world, EntityFactory& factory, const std::string& name, const QJsonObject& json)
{
	std::string templateName = json["template"].toString().toStdString();
	sim::EntityPtr body = factory.createEntity(templateName, name);
	world.addEntity(body);

	readJsonOptionalValue(json, "position", [=](const QJsonValue& object) {
		setPosition(*body, readJsonVector3(object.toArray()));
	});

	readJsonOptionalValue(json, "orientation", [=](const QJsonValue& object) {
		setOrientation(*body, readJsonQuaternion(object.toObject()));
	});

	readJsonOptionalValue(json, "velocity", [=](const QJsonValue& object) {
		setVelocity(*body, readJsonVector3(object.toArray()));
	});

	body->setDynamicsEnabled(json["dynamicsEnabled"].toBool());

	return body;
}

static void loadEntityComponents(World& world, sim::Entity& entity, const QJsonObject& json)
{
	if (auto cameraControllerComponent = entity.getFirstComponent<CameraControllerComponent>())
	{
		QJsonValue value = json["cameraController"];
		if (!value.isUndefined())
		{
			if (auto selector = dynamic_cast<CameraControllerSelector*>(cameraControllerComponent->cameraController.get()))
			{
				loadCameraController(*selector, world, value.toObject());
			}
		}
	}

	skybolt::StringVector attachments = loadEntityAttachments(json["attachments"].toArray());
	for (const std::string& name : attachments)
	{
		findAndAttachEntityByName(entity, world, name);
	}
}

void loadEntities(World& world, EntityFactory& factory, const QJsonValue& value)
{
	assert(!value.isUndefined());

	std::vector<sim::EntityPtr> entities;

	QJsonObject json = value.toObject();
	for (const QString& key : json.keys())
	{
		QJsonObject entity = json.value(key).toObject();
		entities.push_back(loadEntity(world, factory, key.toStdString(), entity));
	}

	int i = 0;
	for (const QString& key : json.keys())
	{
		QJsonObject entity = json.value(key).toObject();
		loadEntityComponents(world, *entities[i], entity);
		++i;
	}
}

static QJsonArray saveEntityAttachments(const Entity& entity)
{
	QJsonArray json;
	for (auto attachment : entity.getComponentsOfType<AttachmentComponent>())
	{
		if (Entity* target = attachment->getTarget())
		{
			json.append(QString::fromStdString(getName(*target)));
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

static QJsonObject saveCameraController(CameraControllerSelector& cameraControllerSelector)
{
	QJsonObject j;
	j["mode"] = QString::fromStdString(cameraControllerSelector.getSelectedControllerName());

	if (cameraControllerSelector.getTarget())
	{
		j["target"] = QString::fromStdString(getName(*cameraControllerSelector.getTarget()));
	}

	QJsonObject modes;
	for (const auto& item : cameraControllerSelector.getControllers())
	{
		QString name = QString::fromStdString(item.first);
		QJsonObject mode;

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

static QJsonObject saveEntity(const Entity& entity, const std::string& templateName)
{
	QJsonObject json;
	json["template"] = QString::fromStdString(templateName);
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

	return json;
}

QJsonObject saveEntities(const World& world)
{
	QJsonObject json;
	for (const EntityPtr& entity : world.getEntities())
	{
		if (!entity->getFirstComponent<ProceduralLifetimeComponent>())
		{
			const std::string& name = getName(*entity);
			auto templateNameComponent = entity->getFirstComponent<TemplateNameComponent>();
			if (!name.empty() && templateNameComponent)
			{
				json[QString::fromStdString(name)] = saveEntity(*entity, templateNameComponent->name);
			}
		}
	}
	return json;
}
