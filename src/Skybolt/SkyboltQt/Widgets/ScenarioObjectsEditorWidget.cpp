/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectsEditorWidget.h"
#include "SkyboltQt/Widgets/ScenarioObjectCreationToolBar.h"
#include "SkyboltQt/Widgets/ScenarioTreeWidget.h"
#include <SkyboltEngine/EngineRoot.h>

#include <QBoxLayout>
#include <QToolBar>

using namespace skybolt;

ScenarioObjectsEditorWidget::ScenarioObjectsEditorWidget(const ScenarioObjectsEditorWidgetConfig& config) :
	QWidget(config.parent)
{
	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);

	QToolBar* toolBar = createScenarioObjectCreationToolBar(config.selectionModel, config.scenarioObjectTypes, config.parent);
	layout->addWidget(toolBar);

	auto scenarioTreeWidget = new ScenarioTreeWidget([&] {
		ScenarioTreeWidgetConfig c;
		c.selectionModel = config.selectionModel,
		c.world = &config.engineRoot->scenario->world;
		c.scenarioObjectTypes = config.scenarioObjectTypes;
		c.contextActions = config.contextActions;
		return c;
	}());

	layout->addWidget(scenarioTreeWidget);
}
