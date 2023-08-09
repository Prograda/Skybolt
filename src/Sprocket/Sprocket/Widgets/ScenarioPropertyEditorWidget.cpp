/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioPropertyEditorWidget.h"
#include "Sprocket/SceneSelectionModel.h"
#include "Sprocket/QtUtil/QtTimerUtil.h"
#include "Sprocket/Widgets/TreeItems.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltSim/World.h>

#include <QBoxLayout>

using namespace skybolt;

ScenarioPropertyEditorWidget::ScenarioPropertyEditorWidget(const ScenarioPropertyEditorWidgetConfig& config) :
	QWidget(config.parent),
	mEngineRoot(config.engineRoot),
	mPropertyModelFactoryMap(config.propertyModelFactoryMap)
{
	assert(mEngineRoot);

	mPropertiesEditor = new PropertyEditor(config.widgetFactoryMap, this);
	mPropertiesEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);
	layout->addWidget(mPropertiesEditor);

	connect(config.selectionModel, &SceneSelectionModel::selectionChanged, this, [this] (const ScenarioObjectPtr& selected, const ScenarioObjectPtr& deselected) {
		selectionChanged(selected, deselected);
	});

	createAndStartIntervalTimer(100, this, [this] {
		if (Updatable* updatablePropertiesModel = dynamic_cast<Updatable*>(mPropertiesModel.get()))
		{
			updatablePropertiesModel->update();
		}
	});
}

void ScenarioPropertyEditorWidget::selectionChanged(const ScenarioObjectPtr& selected, const ScenarioObjectPtr& deselected)
{
	PropertiesModelPtr properties;
	if (selected)
	{
		if (auto i = mPropertyModelFactoryMap.find(typeid(*selected)); i != mPropertyModelFactoryMap.end())
		{
			properties = i->second(*selected);
		}
	}
	setPropertiesModel(properties);
}

void ScenarioPropertyEditorWidget::setPropertiesModel(PropertiesModelPtr properties)
{
	mPropertiesEditor->setModel(nullptr);
	mPropertiesModel = std::move(properties);
	mPropertiesEditor->setModel(mPropertiesModel);
}
