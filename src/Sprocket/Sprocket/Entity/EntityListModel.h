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

#pragma once

#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <QStandardItemModel>

class EntityListModel : public QStandardItemModel, public skybolt::sim::WorldListener
{
public:
	typedef std::function<bool(const skybolt::sim::Entity&)> EntityPredicate;
	EntityListModel(skybolt::sim::World* world, const EntityPredicate& predicate);

	~EntityListModel();

	void setEntityFilter(const EntityPredicate& predicate);

private:
	// WorldListener interface
	void entityAdded(const skybolt::sim::EntityPtr& entity) override;
	void entityAboutToBeRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	void populateList();

private:
	skybolt::sim::World* mWorld;
	EntityPredicate mPredicate;
	std::map<skybolt::sim::Entity*, QStandardItem*> mItems;
};
