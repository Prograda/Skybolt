/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/Scenario/ScenarioObjectTypeMap.h"

class QToolBar;
class QWidget;

QToolBar* createScenarioObjectCreationToolBar(ScenarioSelectionModel* selectionModel, const ScenarioObjectTypeMap& scenarioObjectTypes, QWidget* parent = nullptr);