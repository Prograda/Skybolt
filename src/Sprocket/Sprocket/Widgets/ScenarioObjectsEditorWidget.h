/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/ContextAction/ActionContext.h"
#include "Sprocket/Scenario/ScenarioObject.h"
#include "Sprocket/Scenario/ScenarioObjectTypeMap.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>

#include <QWidget>

struct ScenarioObjectsEditorWidgetConfig
{
	skybolt::EngineRoot* engineRoot;
	ScenarioSelectionModel* selectionModel;
	ScenarioObjectTypeMap scenarioObjectTypes;
	std::vector<DefaultContextActionPtr> contextActions;
	QWidget* parent;
};

class ScenarioObjectsEditorWidget : public QWidget
{
public:
	ScenarioObjectsEditorWidget(const ScenarioObjectsEditorWidgetConfig& config);
};
