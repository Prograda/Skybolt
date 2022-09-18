/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityListModel.h"
#include <SkyboltSim/Components/NameComponent.h>

using namespace skybolt;
using namespace skybolt::sim;

EntityListModel::EntityListModel(World* world, const EntityPredicate& predicate) :
	mWorld(world),
	mPredicate(predicate)
{
	world->addListener(this);

	updateNamesMap();
	updateModel();
}

EntityListModel::~EntityListModel()
{
	for (auto entry : mNamesMap)
	{
		entry.first->removeListener(this);
	}

	mWorld->removeListener(this);
}

void EntityListModel::updateNamesMap()
{
	mNamesMap.clear();

	const sim::World::Entities& entities = mWorld->getEntities();

	for (const sim::EntityPtr& entity : entities)
	{
		if (mPredicate(*entity))
		{
			const std::string& name = getName(*entity);
			if (!name.empty())
			{
				entity->addListener(this);
				mNamesMap[entity.get()] = name;
			}
		}
	}
}

void EntityListModel::setEntityFilter(const EntityPredicate& predicate)
{
	mPredicate = predicate;

	updateNamesMap();
	updateModel();
}

int EntityListModel::rowCount(const QModelIndex& /* parent */) const
{
	return (int)mNames.size();
}

QVariant EntityListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= mNames.size() || index.row() < 0)
	{
		return QVariant();
	}
	if (role == Qt::DisplayRole)
	{
		return QString::fromStdString(mNames.at(index.row()));
	}
	return QVariant();
}

// WorldListener interface
void EntityListModel::entityAdded(const sim::EntityPtr& entity)
{
	if (mPredicate(*entity))
	{
		entity->addListener(this);
		updateName(entity.get());
	}
}

void EntityListModel::entityAboutToBeRemoved(const sim::EntityPtr& entity)
{
	auto it = mNamesMap.find(entity.get());
	if (it != mNamesMap.end())
	{
		mNamesMap.erase(it);
		updateModel();
		entity->removeListener(this);
	}
}

void EntityListModel::onComponentAdded(Entity* entity, Component* component)
{
	updateName(entity);
}

void EntityListModel::onComponentRemove(Entity* entity, Component* component)
{
	updateName(entity);
}

void EntityListModel::updateName(Entity* entity)
{
	const std::string& name = getName(*entity);
	auto it = mNamesMap.find(entity);

	bool changed = false;

	if (name.empty() && it != mNamesMap.end())
	{
		mNamesMap.erase(it);
		changed = true;
	}
	else
	{
		std::string prevName = (it != mNamesMap.end()) ? it->second : "";
		if (name != prevName)
		{
			mNamesMap[entity] = name;
			changed = true;
		}
	}

	if (changed)
	{
		updateModel();
	}
}

void EntityListModel::updateModel()
{
	std::vector<std::string> newNames;
	for (auto entry : mNamesMap)
	{
		newNames.push_back(entry.second);
	}
	std::sort(newNames.begin(), newNames.end());

	if (newNames != mNames)
	{
		beginResetModel();
		mNames = newNames;
		endResetModel();
	}
}
