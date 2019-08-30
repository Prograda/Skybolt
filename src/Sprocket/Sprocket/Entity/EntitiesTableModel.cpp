/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntitiesTableModel.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/NameComponent.h>

using namespace skybolt;

EntitiesTableModel::EntitiesTableModel(QObject *parent, sim::World* world)
	:QAbstractTableModel(parent),
	mWorld(world)
{
	mWorld->addListener(this);

	const sim::World::Entities& entities = mWorld->getEntities();
	for (const sim::EntityPtr& entity : entities)
	{
		mEntities.push_back(entity.get());
	}
}

EntitiesTableModel::~EntitiesTableModel()
{
	mWorld->removeListener(this);
}

int EntitiesTableModel::rowCount(const QModelIndex& parent) const
{
	return (int)mEntities.size();
}

int EntitiesTableModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant EntitiesTableModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		sim::Entity* entity = mEntities[index.row()];
		return QString::fromStdString(getName(*entity));
	}

	return QVariant();
}

sim::Entity* EntitiesTableModel::getEntity(const QModelIndex &index) const
{
	if (index.row() < mEntities.size())
		return mEntities[index.row()];
	return nullptr;
}

QVariant EntitiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case 0:
				return QString("Name");
			}
		}
	}
	return QVariant();
}

void EntitiesTableModel::entityAdded(const sim::EntityPtr& entity)
{
	int index = (int)mEntities.size();
	beginInsertRows(QModelIndex(), index, index);
	mEntities.push_back(entity.get());
	endInsertRows();
}

void EntitiesTableModel::entityAboutToBeRemoved(const sim::EntityPtr& entity)
{
	auto it = std::find(mEntities.begin(), mEntities.end(), entity.get());
	if (it != mEntities.end())
	{
		int index = it - mEntities.begin();
		beginRemoveRows(QModelIndex(), index, index);
		mEntities.erase(it);
		endRemoveRows();
	}
}
