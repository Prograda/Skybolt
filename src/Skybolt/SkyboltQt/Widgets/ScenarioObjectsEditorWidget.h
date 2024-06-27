/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/ContextAction/ActionContext.h"
#include "SkyboltQt/Scenario/ScenarioObject.h"
#include "SkyboltQt/Scenario/ScenarioObjectTypeMap.h"

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
