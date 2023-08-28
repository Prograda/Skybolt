/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectsEditorWidget.h"
#include "Sprocket/Icon/SprocketIcons.h"
#include "Sprocket/Scenario/ScenarioSelectionModel.h"
#include "Sprocket/Widgets/ScenarioTreeWidget.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/EngineRoot.h>

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>

using namespace skybolt;

typedef std::function<bool(const QString&)> StringValidator;

static QString modalNameInputDialog(const QString& defaultName, StringValidator validator)
{
	QDialog dialog(nullptr, Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
	dialog.setWindowTitle("Enter name");

	QVBoxLayout* layout = new QVBoxLayout();
	dialog.setLayout(layout);

	QLineEdit* text = new QLineEdit();
	layout->addWidget(text);

	QLabel* label = new QLabel();
	label->setStyleSheet("QLabel { color : red; }");
	layout->addWidget(label);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	layout->addWidget(buttonBox);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	QObject::connect(text, &QLineEdit::textChanged, [validator, buttonBox, label](const QString& str) {
		bool valid = validator(str);
		buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(valid);
		if (valid)
		{
			label->setText("");
		}
		else
		{
			label->setText("Name already used");
		}
	});
	text->setText(defaultName);

	if (dialog.exec() == QDialog::Accepted)
	{
		return text->text();
	}

	return "";
}

static void connectTreeItemTypeMenuAction(const ScenarioObjectTypePtr& type, QAction* action, const std::string& instanceName, const std::string& templateName = "")
{
	QObject::connect(action, &QAction::triggered, [=]()
	{
		type->objectCreator(instanceName, templateName);
	});
}

static QMenu* createCreateMenu(const ScenarioObjectTypeMap& types)
{
	QMenu* menu = new QMenu();

	for (const auto& [id, type] : types)
	{
		if (type->objectCreator)
		{
			if (type->templateNames.empty())
			{
				QAction* action = new QAction(QString::fromStdString(type->name));
				menu->addAction(action);
				std::string instanceName = type->objectRegistry->createUniqueItemName(type->name);
				connectTreeItemTypeMenuAction(type, action, instanceName);
			}
			else
			{
				QMenu* subMenu = new QMenu(QString::fromStdString(type->name));
				menu->addMenu(subMenu);
				for (const auto& templateName : type->templateNames)
				{
					QAction* action = new QAction(QString::fromStdString(templateName));
					subMenu->addAction(action);
					std::string instanceName = type->objectRegistry->createUniqueItemName(templateName);
					connectTreeItemTypeMenuAction(type, action, instanceName, templateName);
				}
			}
		}
	}

	return menu;
}

static ScenarioObjectTypePtr findScenarioObjectType(const ScenarioObjectTypeMap& scenarioObjectTypes, const ScenarioObject& object)
{
	std::type_index type = typeid(object);
	return findOptional(scenarioObjectTypes, type).value_or(nullptr);
}

ScenarioObjectsEditorWidget::ScenarioObjectsEditorWidget(const ScenarioObjectsEditorWidgetConfig& config) :
	mEngineRoot(config.engineRoot),
	QWidget(config.parent)
{
	assert(mEngineRoot);

	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);

	QToolBar* toolbar = new QToolBar(this);

	QToolButton* createButton = new QToolButton(this);
	createButton->setIcon(getSprocketIcon(SprocketIcon::Add));
	toolbar->addWidget(createButton);

	QToolButton* deleteButton = new QToolButton(this);
	deleteButton->setIcon(getSprocketIcon(SprocketIcon::Remove));
	deleteButton->setEnabled(false);
	toolbar->addWidget(deleteButton);

	QMenu* menu = createCreateMenu(config.scenarioObjectTypes);

	createButton->setMenu(menu);
	createButton->setPopupMode(QToolButton::InstantPopup);

	layout->addWidget(toolbar);

	QObject::connect(config.selectionModel, &ScenarioSelectionModel::selectionChanged, this, [deleteButton, scenarioObjectTypes = config.scenarioObjectTypes]
		(const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected)
	{
		bool enabled = false;
		if (auto item = getFirstSelectedScenarioObject(selected); item)
		{
			if (const ScenarioObjectTypePtr& type = findScenarioObjectType(scenarioObjectTypes, *item); type)
			{
				enabled = type->isObjectRemovable(*item);
			}
		}
		deleteButton->setEnabled(enabled);
	});

	QObject::connect(deleteButton, &QToolButton::pressed, [selectionModel = config.selectionModel, scenarioObjectTypes = config.scenarioObjectTypes]()
	{
		if (const auto& object = getFirstSelectedScenarioObject(selectionModel->getSelectedItems()); object)
		{
			if (const ScenarioObjectTypePtr& type = findScenarioObjectType(scenarioObjectTypes, *object); type)
			{
				if (type && type->isObjectRemovable(*object))
				{
					type->objectRemover(*object);
				}
			}
		}
	});

	mScenarioTreeWidget = new ScenarioTreeWidget([&] {
		ScenarioTreeWidgetConfig c;
		c.selectionModel = config.selectionModel,
		c.world = &mEngineRoot->scenario->world;
		c.scenarioObjectTypes = config.scenarioObjectTypes;
		c.contextActions = config.contextActions;
		return c;
	}());

	layout->addWidget(mScenarioTreeWidget);
}
