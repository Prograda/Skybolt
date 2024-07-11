/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityChooserDialogFactory.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/World.h>

#include <QListWidget>

#include <assert.h>

using namespace skybolt;

EntityChooserDialogFactory::EntityChooserDialogFactory(const skybolt::sim::World* world) :
	mWorld(world)
{
	assert(mWorld);
}

skybolt::sim::EntityPtr EntityChooserDialogFactory::chooseEntity() const
{
	std::map<QString, sim::EntityPtr> entityNames;
	QStringList items;
	for (const auto& entity : mWorld->getEntities())
	{
		auto name = sim::getName(*entity);
		if (!name.empty())
		{
			QString qtName = QString::fromStdString(name);
			entityNames[qtName] = entity;
			items.push_back(qtName);
		}
	}
	std::sort(items.begin(), items.end());

	QListWidget* list = new QListWidget();
	list->addItems(items);

	std::shared_ptr<QDialog> dialog = createDialogModal(list, "Choose Entity");
	if (dialog->exec() == QDialog::Accepted)
	{
		QModelIndex index = list->currentIndex();
		if (index.isValid())
		{
			return entityNames[list->item(index.row())->text()];
		}
	}
	return nullptr;
}