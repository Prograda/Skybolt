/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "TreeItems.h"
#include "Sprocket/ContextAction/ActionContext.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

#include <QWidget>

class QTreeView;

struct WorldTreeWidgetConfig
{
	skybolt::sim::World* world;
	skybolt::EntityFactory* factory;
	std::vector<TreeItemType> itemTypes;
	skybolt::Scenario* scenario;
	std::vector<DefaultContextActionPtr> contextActions;
};

class WorldTreeWidget : public QWidget, skybolt::sim::WorldListener, skybolt::sim::EntityListener
{
	Q_OBJECT

	friend struct WorldTreeWidgetRegistryListener;

public:
	WorldTreeWidget(const WorldTreeWidgetConfig& config);
	~WorldTreeWidget();
	
	void setSelectedEntity(const skybolt::sim::Entity* entity);

signals:
	void selectionChanged(const TreeItem&);
	void itemClicked(const TreeItem&);

private:
	void setItemsUnderParent(TreeItem& parent, const Registry<TreeItem>& registry);
	const TreeItemType* findItemType(const TreeItem& item) const; //!< Returns nullptr if no type has not been registered for the item
	bool isDeletable(const TreeItem& item) const;
	void showContextMenu(TreeItem& item, const QPoint& point);

	void updateTreeItemParents();
	TreeItemPtr getParentTreeItem(const skybolt::sim::Entity& entity) const;

	std::vector<TreeItem*> getCurrentSelection() const;
	void setCurrentSelection(std::vector<TreeItem*> items);

private:
	// WorldListener interface
	void entityAdded(const skybolt::sim::EntityPtr& entity) override;
	void entityRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	// EntityListener interface
	void onComponentAdded(skybolt::sim::Entity* entity, skybolt::sim::Component* component) override;
	void onComponentRemove(skybolt::sim::Entity* entity, skybolt::sim::Component* component) override;

private:
	skybolt::sim::World* mWorld;
	const std::vector<DefaultContextActionPtr> mContextActions;
	std::vector<TreeItemType> mItemTypes;
	TreeItemModel* mModel;
	QTreeView* mView;
	TreeItemPtr mEntityRootItem;
	std::vector<TreeItemPtr> mTypeRootItems;
	std::vector<std::unique_ptr<struct WorldTreeWidgetRegistryListener>> mRegistryListeners;
	
	std::map<const skybolt::sim::Entity*, TreeItemPtr> mEntityTreeItems;
};
