/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequenceSerializer.h"

#include <Sprocket/JsonHelpers.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <SkyboltEngine/Sequence/JulianDateSequenceController.h>
#include <SkyboltSim/Components/NameComponent.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

using namespace skybolt;

static EntitySequenceState readEntityState(const QJsonObject& object)
{
	EntitySequenceState s;
	s.position = readJsonVector3(object["position"].toArray());
	s.orientation = readJsonQuaternion(object["orientation"].toObject());
	return s;
}

static std::shared_ptr<EntityStateSequence> readEntityStateSequence(const QJsonObject& object)
{
	auto sequence = std::make_shared<EntityStateSequence>();
	for (const auto& time : object.keys())
	{
		QJsonObject itemValue = object[time].toObject();
		EntitySequenceState value = readEntityState(itemValue);
		sequence->addItemAtTime(value, time.toDouble());
	}
	return sequence;
}

std::shared_ptr<EntityStateSequenceController> readEntityStateSequenceController(const QJsonObject& object, const sim::World& world)
{
	auto controller = std::make_shared<EntityStateSequenceController>(readEntityStateSequence(object["sequence"].toObject()));

	auto it = object.find("entityName");
	if (it != object.end())
	{
		QString entityName = it.value().toString();
		sim::EntityPtr entity = findObjectByName(world, entityName.toStdString());
		if (entity)
		{
			controller->setEntity(entity.get());
		}
	}

	return controller;
}

static std::shared_ptr<DoubleStateSequence> readDoubleSequence(const QJsonObject& object)
{
	auto sequence = std::make_shared<DoubleStateSequence>();
	for (const auto& time : object.keys())
	{
		DoubleSequenceState value = object[time].toDouble();
		sequence->addItemAtTime(value, time.toDouble());
	}
	return sequence;
}

static std::shared_ptr<JulianDateSequenceController> readJulianDateSequenceController(const QJsonObject& object, Scenario* scenario)
{
	return std::make_shared<JulianDateSequenceController>(readDoubleSequence(object["sequence"].toObject()), scenario);
}

StateSequenceControllerPtr readSequenceController(const QJsonObject& object, const sim::World& world, Scenario* scenario)
{
	StateSequenceControllerPtr controller;

	QString type = object["type"].toString();
	if (type == "EntityState")
	{
		controller = readEntityStateSequenceController(object, world);
	}
	else if (type == "JulianDate")
	{
		controller = readJulianDateSequenceController(object, scenario);
	}
	else
	{
		throw Exception("Unsupported Sequence type: " + type.toStdString());
	}
	return controller;
}

static QJsonObject writeEntityState(const EntitySequenceState& state)
{
	QJsonObject object;
	object["position"] = writeJson(state.position);
	object["orientation"] = writeJson(state.orientation);
	return object;
}

static QJsonObject writeEntityStateSequence(const EntityStateSequence& sequence)
{
	QJsonObject object;

	assert(sequence.times.size() == sequence.values.size());
	for (size_t i = 0; i < sequence.times.size(); ++i)
	{
		object[QString::number(sequence.times[i])] = writeEntityState(sequence.values[i]);
	}
	return object;
}

static QJsonObject writeEntityStateSequenceController(const EntityStateSequenceController& controller)
{
	QJsonObject object;
	object["type"] = "EntityState";
	object["sequence"] = writeEntityStateSequence(static_cast<const EntityStateSequence&>(*controller.getSequence()));

	auto entity = controller.getEntity();
	if (entity)
	{
		std::string name = sim::getName(*entity);
		if (!name.empty())
		{
			object["entityName"] = QString::fromStdString(name);
		}
	}
	return object;
}

static QJsonObject writeDoubleStateSequence(const DoubleStateSequence& sequence)
{
	QJsonObject object;

	assert(sequence.times.size() == sequence.values.size());
	for (size_t i = 0; i < sequence.times.size(); ++i)
	{
		object[QString::number(sequence.times[i])] = sequence.values[i].value;
	}
	return object;
}

static QJsonObject writeJulianDateSequenceController(const JulianDateSequenceController& controller)
{
	QJsonObject object;
	object["type"] = "JulianDate";
	object["sequence"] = writeDoubleStateSequence(static_cast<const DoubleStateSequence&>(*controller.getSequence()));
	return object;
}

QJsonObject writeSequenceController(const StateSequenceController& controller)
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
	return QJsonObject();
}