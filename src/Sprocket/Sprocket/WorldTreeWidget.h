/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "ContextAction.h"
#include "Registry.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

#include <QWidget>

template <class T>
struct TreeItemT : public TreeItem
{
	QString label;
	T data;
	
	TreeItemT(const QIcon& icon, const QString& label, const T& data)
		: TreeItem(icon), label(label), data(data)
	{
	}

	const QString& getLabel() override
	{
		return label;
	}
};

typedef TreeItemT<void*> SimpleTreeItem;
typedef TreeItemT<skybolt::sim::Entity*> EntityTreeItem;
typedef TreeItemT<skybolt::Scenario*> ScenarioTreeItem;

struct TreeItemType
{
	TreeItemType(const std::type_index& itemTypeId) : itemTypeId(itemTypeId) {}

	std::string name;
	std::vector<std::string> subTypes; //!< Names of each sub type of this item. Leave empty if there's only one type.
	std::function<void(const std::string& name, const std::string& subType)> itemCreator;
	std::function<void(TreeItem*)> itemDeleter; //!< Throws exception if item is not disposable
	std::shared_ptr<Registry<TreeItem>> itemRegistry;
	std::type_index itemTypeId;
};

typedef std::shared_ptr<ContextAction<TreeItem>> TreeItemContextActionPtr;

struct WorldTreeWidgetConfig
{
	skybolt::sim::World* world;
	skybolt::EntityFactory* factory;
	std::vector<TreeItemType> itemTypes;
	skybolt::Scenario* scenario;
	std::vector<TreeItemContextActionPtr> contextActions;
};

class WorldTreeWidget : public QWidget, skybolt::sim::WorldListener, skybolt::sim::EntityListener
{
	Q_OBJECT

	friend struct WorldTreeWidgetRegistryListener;

public:
	WorldTreeWidget(const WorldTreeWidgetConfig& config);
	~WorldTreeWidget();

	void update();
	
signals:
	void selectionChanged(const TreeItem&);
	void itemClicked(const TreeItem&);

private:
	void updateEntityItems();
	void setItemsUnderParent(TreeItem& parent, const Registry<TreeItem>& registry);
	const TreeItemType* findItemType(const TreeItem& item) const; //!< Returns nullptr if no type has not been registered for the item
	bool isDeletable(const TreeItem& item) const;
	void showContextMenu(TreeItem& item, const QPoint& point);

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
	const std::vector<TreeItemContextActionPtr> mContextActions;
	std::vector<TreeItemType> mItemTypes;
	TreeItemModel* mModel;
	TreeItemPtr mEntityRootItem;
	std::vector<TreeItemPtr> mTypeRootItems;
	std::vector<std::unique_ptr<struct WorldTreeWidgetRegistryListener>> mRegistryListeners;
	bool mDirty = false;
};
