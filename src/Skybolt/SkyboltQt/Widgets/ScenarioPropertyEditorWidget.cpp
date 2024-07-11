/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioPropertyEditorWidget.h"
#include "SkyboltQt/QtUtil/QtScrollAreaUtil.h"
#include "SkyboltQt/QtUtil/QtTimerUtil.h"
#include "SkyboltQt/Widgets/TreeItemModel.h"

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
	layout->addWidget(wrapWithVerticalScrollBar(mPropertiesEditor, this));

	connect(config.selectionModel, &ScenarioSelectionModel::selectionChanged, this, [this] (const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected) {
		selectionChanged(selected, deselected);
	});

	createAndStartIntervalTimer(100, this, [this] {
		if (Updatable* updatablePropertiesModel = dynamic_cast<Updatable*>(mPropertiesModel.get()))
		{
			updatablePropertiesModel->update();
		}
	});
}

void ScenarioPropertyEditorWidget::selectionChanged(const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected)
{
	PropertiesModelPtr properties;
	if (auto object = getFirstSelectedScenarioObject(selected); object)
	{
		if (auto i = mPropertyModelFactoryMap.find(typeid(*object)); i != mPropertyModelFactoryMap.end())
		{
			properties = i->second(*object);
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
