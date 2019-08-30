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

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <QJsonObject>

void loadScenario(skybolt::Scenario& scenario, const QJsonObject& object);

QJsonObject saveScenario(const skybolt::Scenario& scenario);

void loadEntities(skybolt::sim::World& world, skybolt::EntityFactory& factory, const QJsonValue& value);

QJsonObject saveEntities(const skybolt::sim::World& world);
