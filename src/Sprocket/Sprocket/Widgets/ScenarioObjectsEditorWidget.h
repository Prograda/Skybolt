/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/ContextAction/ActionContext.h"
#include "Sprocket/Scenario/ScenarioObject.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>

#include <QWidget>

struct ScenarioObjectsEditorWidgetConfig
{
	skybolt::EngineRoot* engineRoot;
	SceneSelectionModel* selectionModel;
	ScenarioObjectTypeMap scenarioObjectTypes;
	std::vector<DefaultContextActionPtr> contextActions;
	QWidget* parent;
};

class ScenarioObjectsEditorWidget : public QWidget
{
public:
	ScenarioObjectsEditorWidget(const ScenarioObjectsEditorWidgetConfig& config);

	void selectionChanged(const ScenarioObjectPtr& selected, const ScenarioObjectPtr& deselected);

private:
	skybolt::EngineRoot* mEngineRoot;
	ScenarioTreeWidget* mScenarioTreeWidget;
};
