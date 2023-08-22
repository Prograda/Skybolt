/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "Sprocket/Registry.h"
#include "Sprocket/SprocketFwd.h"
#include "Sprocket/ContextAction/ActionContext.h"
#include "Sprocket/Scenario/ScenarioObject.h"

#include <QWidget>

class QTreeView;

struct ScenarioTreeWidgetConfig
{
	SceneSelectionModel* selectionModel;
	skybolt::sim::World* world;
	ScenarioObjectTypeMap scenarioObjectTypes;
	std::vector<DefaultContextActionPtr> contextActions;
};

struct ScenarioObjectTreeItem;

class ScenarioTreeWidget : public QWidget
{
	Q_OBJECT

	friend struct ScenarioObjectRegistryListener;

public:
	ScenarioTreeWidget(const ScenarioTreeWidgetConfig& config);
	~ScenarioTreeWidget();

private:
	void addObjects(const std::type_index& sceneObjectType, const std::set<ScenarioObjectPtr>& objects);
	void removeObjects(const std::type_index& sceneObjectType, const std::set<ScenarioObjectPtr>& objects);

	ScenarioObjectPtr findScenarioObject(const TreeItem& item) const; //!< Returns nullptr if item has no scenario object
	ActionContext toActionContext(const skybolt::sim::World& world, const TreeItem& item) const;
	void showContextMenu(TreeItem& item, const QPoint& point);

	TreeItemPtr getParent(const ScenarioObject& object) const;
	void updateTreeItemParents();

	std::vector<TreeItem*> getCurrentSelection() const;
	void setCurrentSelection(const std::vector<TreeItem*>& items);

private:
	skybolt::sim::World* mWorld;
	const std::vector<DefaultContextActionPtr> mContextActions;
	ScenarioObjectTypeMap mScenarioObjectTypes;
	TreeItemModel* mModel;
	QTreeView* mView;
	std::map<std::type_index, TreeItemPtr> mScenarioObjectTypeToTreeParentMap;
	std::vector<std::unique_ptr<struct ScenarioObjectRegistryListener>> mRegistryListeners;

	std::map<ScenarioObjectPtr, std::shared_ptr<ScenarioObjectTreeItem>> mItemsMap;
};