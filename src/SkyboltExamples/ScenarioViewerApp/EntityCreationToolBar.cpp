/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityCreationToolBar.h"
#include <SkyboltEngineQt/Icon/SkyboltIcons.h>
#include <SkyboltWidgets/Util/QtMenuUtil.h>

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltSim/World.h>

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

static QAction* addCreateObjectSubMenu(QMenu& parentMenu, const skybolt::ScenarioObjectPath& directory, const std::string& name)
{
	// Create menu hierarchy
	QMenu* currentParentMenu = &parentMenu;
	for (const std::string& subMenuName : directory)
	{
		QString submenuNameQt = QString::fromStdString(subMenuName);
		QMenu* subMenu = findSubMenuByName(*currentParentMenu, submenuNameQt);
		if (!subMenu)
		{
			subMenu = new QMenu(submenuNameQt, currentParentMenu);
			currentParentMenu->addMenu(subMenu);
		}
		currentParentMenu = subMenu;
	}

	// Add action
	QAction* action = new QAction(QString::fromStdString(name), currentParentMenu);
	currentParentMenu->addAction(action);
	return action;
}

static QMenu* createCreateMenu(const EntityCreationToolBarConfig& config)
{
	QMenu* menu = new QMenu(config.parent);

	for (const auto& templateName : config.entityFactory->getTemplateNames())
	{
		ScenarioObjectPath path = config.entityFactory->getScenarioObjectDirectoryForTemplate(templateName);
		QAction* action = addCreateObjectSubMenu(*menu, path, templateName);

		QObject::connect(action, &QAction::triggered, [=]()
		{
			sim::EntityPtr entity = config.entityFactory->createEntity(templateName);
			
			if (config.onEntityCreated)
			{
				config.onEntityCreated(*entity);
			}

			config.world->addEntity(entity);
		});
	}

	return menu;
}

EntityCreationToolBar::EntityCreationToolBar(const EntityCreationToolBarConfig& config) :
	QToolBar(config.parent)
{
	QToolButton* createButton = new QToolButton(config.parent);
	createButton->setToolTip("Add item");
	createButton->setText("+");
	createButton->setIcon(getSkyboltIcon(SkyboltIcon::Add));
	addWidget(createButton);

	mDeleteButton = new QToolButton(config.parent);
	mDeleteButton->setToolTip("Remove item");
	mDeleteButton->setText("-");
	mDeleteButton->setIcon(getSkyboltIcon(SkyboltIcon::Remove));
	mDeleteButton->setEnabled(false);
	addWidget(mDeleteButton);

	QMenu* menu = createCreateMenu(config);
	createButton->setMenu(menu);
	createButton->setPopupMode(QToolButton::InstantPopup);

	QObject::connect(mDeleteButton, &QToolButton::clicked, config.parent, [this, world = config.world]()
	{
		sim::EntityPtr entity = world->getEntityById(mSelectedEntityId);
		if (entity)
		{
			world->removeEntity(entity.get());
		}
	});
}

void EntityCreationToolBar::setSelectedEntity(const skybolt::sim::EntityId& entityId)
{
	mDeleteButton->setEnabled(entityId != sim::nullEntityId());
	mSelectedEntityId = entityId;
}