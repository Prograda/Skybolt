/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequenceSerializer.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <SkyboltEngine/Sequence/JulianDateSequenceController.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/Components/NameComponent.h>

using namespace skybolt;

static EntitySequenceState readEntityState(const nlohmann::json& json)
{
	EntitySequenceState s;
	s.position = sim::readVector3(json.at("position"));
	s.orientation = sim::readQuaternion(json.at("orientation"));
	return s;
}

static std::shared_ptr<EntityStateSequence> readEntityStateSequence(const nlohmann::json& json)
{
	auto sequence = std::make_shared<EntityStateSequence>();
	for (const auto& i : json.items())
	{
		EntitySequenceState value = readEntityState(i.value());
		sequence->addItemAtTime(value, stod(i.key()));
	}
	return sequence;
}

std::shared_ptr<EntityStateSequenceController> readEntityStateSequenceController(const nlohmann::json& json, const sim::World& world)
{
	auto controller = std::make_shared<EntityStateSequenceController>(readEntityStateSequence(json.at("sequence")));

	ifChildExists(json, "entityName", [&] (const nlohmann::json& child) {
		std::string entityName = child.get<std::string>();
		sim::Entity* entity = world.findObjectByName(entityName).get();
		if (entity)
		{
			controller->setEntity(entity);
		}
	});

	return controller;
}

static std::shared_ptr<DoubleStateSequence> readDoubleSequence(const nlohmann::json& json)
{
	auto sequence = std::make_shared<DoubleStateSequence>();
	for (const auto& i : json.items())
	{
		DoubleSequenceState value(double(i.value()));
		sequence->addItemAtTime(value, stod(i.key()));
	}
	return sequence;
}

static std::shared_ptr<JulianDateSequenceController> readJulianDateSequenceController(const nlohmann::json& json, Scenario* scenario)
{
	return std::make_shared<JulianDateSequenceController>(readDoubleSequence(json.at("sequence")), scenario);
}

StateSequenceControllerPtr readSequenceController(const nlohmann::json& json, Scenario* scenario)
{
	StateSequenceControllerPtr controller;

	std::string type = json.at("type");
	if (type == "EntityState")
	{
		controller = readEntityStateSequenceController(json, scenario->world);
	}
	else if (type == "JulianDate")
	{
		controller = readJulianDateSequenceController(json, scenario);
	}
	else
	{
		throw Exception("Unsupported Sequence type: " + type);
	}
	return controller;
}

static nlohmann::json writeEntityState(const EntitySequenceState& state)
{
	nlohmann::json object;
	object["position"] = sim::writeJson(state.position);
	object["orientation"] = sim::writeJson(state.orientation);
	return object;
}

static nlohmann::json writeEntityStateSequence(const EntityStateSequence& sequence)
{
	nlohmann::json object;

	assert(sequence.times.size() == sequence.values.size());
	for (size_t i = 0; i < sequence.times.size(); ++i)
	{
		object[std::to_string(sequence.times[i])] = writeEntityState(sequence.values[i]);
	}
	return object;
}

static nlohmann::json writeEntityStateSequenceController(const EntityStateSequenceController& controller)
{
	nlohmann::json object;
	object["type"] = "EntityState";
	object["sequence"] = writeEntityStateSequence(static_cast<const EntityStateSequence&>(*controller.getSequence()));

	auto entity = controller.getEntity();
	if (entity)
	{
		std::string name = sim::getName(*entity);
		if (!name.empty())
		{
			object["entityName"] = name;
		}
	}
	return object;
}

static nlohmann::json writeDoubleStateSequence(const DoubleStateSequence& sequence)
{
	nlohmann::json object;

	assert(sequence.times.size() == sequence.values.size());
	for (size_t i = 0; i < sequence.times.size(); ++i)
	{
		object[std::to_string(sequence.times[i])] = sequence.values[i].value;
	}
	return object;
}

static nlohmann::json writeJulianDateSequenceController(const JulianDateSequenceController& controller)
{
	nlohmann::json object;
	object["type"] = "JulianDate";
	object["sequence"] = writeDoubleStateSequence(static_cast<const DoubleStateSequence&>(*controller.getSequence()));
	return object;
}

nlohmann::json writeSequenceController(const StateSequenceController& controller)
{
	if (const EntityStateSequenceController* entityStateSequenceController = dynamic_cast<const EntityStateSequenceController*>(&controller))
	{
		return writeEntityStateSequenceController(*entityStateSequenceController);
	}
	else if (const JulianDateSequenceController* julianDateSequenceController = dynamic_cast<const JulianDateSequenceController*>(&controller))
	{
		return writeJulianDateSequenceController(*julianDateSequenceController);
	}
	assert(!"Unsupported sequence type");
	return nlohmann::json();
}