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

EntityListModel::EntityListModel(World* world, const EntityPredicate& predicate, bool addBlankItem) :
	mWorld(world),
	mAddBlankItem(addBlankItem),
	mPredicate(predicate)
{
	world->addListener(this);
	populateList();
}

EntityListModel::~EntityListModel()
{
	mWorld->removeListener(this);
}


void EntityListModel::setEntityFilter(const EntityPredicate& predicate)
{
	mPredicate = predicate;
	populateList();
}

// WorldListener interface
void EntityListModel::entityAdded(const sim::EntityPtr& entity)
{
	populateList();
}

void EntityListModel::entityAboutToBeRemoved(const sim::EntityPtr& entity)
{
	populateList();
}

std::optional<QString> EntityListModel::toItem(const sim::Entity& entity) const
{
	if (mPredicate(entity))
	{
		if (std::string name = getName(entity); !name.empty())
		{
			return QString::fromStdString(name);
		}
	}
	return std::nullopt;
}

void EntityListModel::populateList()
{
	// Make a complete list of items we want in the list
	std::set<QString> newItems;
	if (mAddBlankItem)
	{
		newItems.insert("");
	}

	for (const auto& entity : mWorld->getEntities())
	{
		if (auto item = toItem(*entity); item)
		{
			newItems.insert(*item);
		}
	}

	// Add new items
	for (const QString& item : newItems)
	{
		if (mItems.find(item) == mItems.end())
		{
			insertRow(rowCount(), new QStandardItem(item));
		}
	}

	// Remove old items
	for (const QString& item : mItems)
	{
		if (newItems.find(item) == newItems.end())
		{
			if (auto items = findItems(item); !items.empty())
			{
				removeRow(items.front()->row());
			}
		}
	}
	std::swap(mItems, newItems);
}