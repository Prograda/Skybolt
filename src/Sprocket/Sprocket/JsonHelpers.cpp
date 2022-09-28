/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "JsonHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <QJsonArray>
#include <stdexcept>

using namespace skybolt;

sim::Vector3 readJsonVector3(const QJsonArray& v)
{
	if (v.size() != 3)
	{
		throw std::runtime_error("Incorrect number of elements in vector. Expected 3.");
	}
	return sim::Vector3(v[0].toDouble(), v[1].toDouble(), v[2].toDouble());
}

sim::Quaternion readJsonQuaternion(const QJsonObject& object)
{
	double angleRad = object["angleDeg"].toDouble() * skybolt::math::degToRadD();
	sim::Vector3 axis = readJsonVector3(object["axis"].toArray());
	return glm::angleAxis(angleRad, axis);
}

void readJsonOptionalValue(const QJsonObject& object, const QString& name, std::function<void(const QJsonValue& object)> lambda)
{
	auto v = object.find(name);
	if (v != object.end())
	{
		lambda(v.value());
	}
}

void writeIfNotEmpty(QJsonObject& object, const QString& key, const QJsonArray& array)
{
	if (!array.isEmpty())
	{
		object[key] = array;
	}
}

QJsonArray writeJson(const sim::Vector3& v)
{
	return { v.x, v.y, v.z };
}

QJsonObject writeJson(const sim::Quaternion& v)
{
	QJsonObject r;
	r["angleDeg"] = glm::angle(v) * skybolt::math::radToDegD();
	r["axis"] = writeJson(glm::axis(v));
	return r;
}