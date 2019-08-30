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
#include <QAbstractListModel>

// TODO: simplify this. Should be using StandardItemListModel and a sort filter proxy model.
// Fix bug where current selection is lost when list changes.
class EntityListModel : public QAbstractListModel, public skybolt::sim::WorldListener, public skybolt::sim::EntityListener
{
public:
	typedef std::function<bool(const skybolt::sim::Entity&)> EntityPredicate;
	EntityListModel(skybolt::sim::World* world, const EntityPredicate& predicate);

	~EntityListModel();

	void setEntityFilter(const EntityPredicate& predicate);

	int rowCount(const QModelIndex& /* parent */) const;

	QVariant data(const QModelIndex &index, int role) const;

private:
	// WorldListener interface
	void entityAdded(const skybolt::sim::EntityPtr& entity) override;

	void entityAboutToBeRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	// EntityListener interface
	void onComponentAdded(skybolt::sim::Entity* entity, skybolt::sim::Component* component) override;

	void onComponentRemove(skybolt::sim::Entity* entity, skybolt::sim::Component* component) override;

	void updateName(skybolt::sim::Entity* entity);

	void updateModel();

	void updateNamesMap();

private:
	skybolt::sim::World* mWorld;
	EntityPredicate mPredicate;
	std::map<skybolt::sim::Entity*, std::string> mNamesMap;
	std::vector<std::string> mNames;
};
