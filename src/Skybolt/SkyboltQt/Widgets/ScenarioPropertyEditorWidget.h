/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/Property/PropertyEditor.h"
#include "SkyboltQt/Property/PropertyModelFactoryMap.h"
#include "SkyboltQt/Scenario/ScenarioSelectionModel.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>

struct ScenarioPropertyEditorWidgetConfig
{
	skybolt::EngineRoot* engineRoot;
	ScenarioSelectionModel* selectionModel;
	PropertyModelFactoryMap propertyModelFactoryMap;
	PropertyEditorWidgetFactoryMap widgetFactoryMap;
	QWidget* parent = nullptr;
};

class ScenarioPropertyEditorWidget : public QWidget
{
public:
	ScenarioPropertyEditorWidget(const ScenarioPropertyEditorWidgetConfig& config);

private:
	void setPropertiesModel(PropertiesModelPtr properties);

	void selectionChanged(const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected);

private:
	skybolt::EngineRoot* mEngineRoot;
	PropertyModelFactoryMap mPropertyModelFactoryMap;
	PropertyEditor* mPropertiesEditor = nullptr;
	PropertiesModelPtr mPropertiesModel;
};
