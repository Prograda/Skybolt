/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectCreationToolBar.h"
#include "SkyboltQt/Scenario/ScenarioObject.h"
#include "SkyboltQt/Icon/SkyboltIcons.h"
#include "SkyboltQt/Scenario/ScenarioSelectionModel.h"

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

static void connectTreeItemTypeMenuAction(const ScenarioObjectTypePtr& type, QAction* action, const std::string& instanceBaseName, const std::string& templateName = "")
{
	QObject::connect(action, &QAction::triggered, [=]()
	{
		std::string instanceName = type->objectRegistry->createUniqueItemName(instanceBaseName);
		type->objectCreator(instanceName, templateName);
	});
}

static QMenu* createCreateMenu(const ScenarioObjectTypeMap& types, QWidget* parent)
{
	QMenu* menu = new QMenu(parent);

	for (const auto& [id, type] : types)
	{
		if (type->objectCreator)
		{
			if (type->templateNames.empty())
			{
				QAction* action = new QAction(QString::fromStdString(type->name), menu);
				menu->addAction(action);
				connectTreeItemTypeMenuAction(type, action, type->name);
			}
			else
			{
				QMenu* subMenu = new QMenu(QString::fromStdString(type->name), menu);
				menu->addMenu(subMenu);
				for (const auto& templateName : type->templateNames)
				{
					QAction* action = new QAction(QString::fromStdString(templateName), menu);
					subMenu->addAction(action);
					connectTreeItemTypeMenuAction(type, action, templateName, templateName);
				}
			}
		}
	}

	return menu;
}

static ScenarioObjectTypePtr findScenarioObjectType(const ScenarioObjectTypeMap& scenarioObjectTypes, const ScenarioObject& object)
{
	std::type_index type = typeid(object);
	return skybolt::findOptional(scenarioObjectTypes, type).value_or(nullptr);
}

QToolBar* createScenarioObjectCreationToolBar(ScenarioSelectionModel* selectionModel, const ScenarioObjectTypeMap& scenarioObjectTypes, QWidget* parent)
{
	QToolBar* toolbar = new QToolBar(parent);

	QToolButton* createButton = new QToolButton(parent);
	createButton->setToolTip("Add item");
	createButton->setText("+");
	createButton->setIcon(getSkyboltIcon(SkyboltIcon::Add));
	toolbar->addWidget(createButton);

	QToolButton* deleteButton = new QToolButton(parent);
	deleteButton->setToolTip("Remove item");
	deleteButton->setText("-");
	deleteButton->setIcon(getSkyboltIcon(SkyboltIcon::Remove));
	deleteButton->setEnabled(false);
	toolbar->addWidget(deleteButton);

	QMenu* menu = createCreateMenu(scenarioObjectTypes, parent);
	createButton->setMenu(menu);
	createButton->setPopupMode(QToolButton::InstantPopup);

	QObject::connect(selectionModel, &ScenarioSelectionModel::selectionChanged, parent, [deleteButton, scenarioObjectTypes]
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

	QObject::connect(deleteButton, &QToolButton::pressed, parent, [selectionModel, scenarioObjectTypes]()
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

	return toolbar;
}