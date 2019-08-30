/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/World.h>
#include <QJsonObject>
#include <memory>

skybolt::StateSequenceControllerPtr readSequenceController(const QJsonObject& object, const skybolt::sim::World& world, skybolt::Scenario* scenario);

QJsonObject writeSequenceController(const skybolt::StateSequenceController& controller);