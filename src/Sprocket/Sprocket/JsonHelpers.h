/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SimMath.h>
#include <QJsonObject>
#include <functional>

skybolt::sim::Vector3 readJsonVector3(const QJsonArray& v);

skybolt::sim::Quaternion readJsonQuaternion(const QJsonObject& object);

void readJsonOptionalValue(const QJsonObject& object, const QString& name, std::function<void(const QJsonValue& object)> lambda);

void writeIfNotEmpty(QJsonObject& object, const QString& key, const QJsonArray& array);

QJsonArray writeJson(const skybolt::sim::Vector3& v);

QJsonObject writeJson(const skybolt::sim::Quaternion& v);