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
	if (mPredicate(*entity))
	{
		if (std::string name = getName(*entity); !name.empty())
		{
			auto item = new QStandardItem(QString::fromStdString(name));
			appendRow(item);
			mItems[entity.get()] = item;
		}
	}
}

void EntityListModel::entityAboutToBeRemoved(const sim::EntityPtr& entity)
{
	if (auto i = mItems.find(entity.get()); i != mItems.end())
	{
		removeRow(i->second->row());
		mItems.erase(i);
	}
}

void EntityListModel::populateList()
{
	clear();
	mItems.clear();
	for (const auto& entity : mWorld->getEntities())
	{
		entityAdded(entity);
	}
}