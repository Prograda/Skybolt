/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/ContextAction/ActionContext.h"
#include "SkyboltQt/Scenario/ScenarioObject.h"
#include "SkyboltQt/Scenario/ScenarioObjectTypeMap.h"

#include <QWidget>

class QTreeView;

struct ScenarioTreeWidgetConfig
{
	ScenarioSelectionModel* selectionModel;
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

protected:
	virtual bool shouldDisplayItem(const ScenarioObject& object) const;

private:
	void addOrRemoveObjects();
	void addObjects(const std::set<ScenarioObjectPtr>& objects);
	void removeObjects(const std::set<ScenarioObjectPtr>& objects);

	ScenarioObjectPtr findScenarioObject(const TreeItem& item) const; //!< Returns nullptr if item has no scenario object
	ActionContext toActionContext(const skybolt::sim::World& world, const TreeItem& item) const;
	void showContextMenu(TreeItem& item, const QPoint& point);

	TreeItemPtr createFolder(TreeItem& parent, const std::string& name);

	TreeItemPtr getParent(const ScenarioObject& object); //!< Never returns null
	void updateTreeItemParents();
	void updateItemDisplayNames();

	std::vector<TreeItem*> getCurrentSelection() const;
	void setCurrentSelection(const std::vector<TreeItem*>& items);

protected:
	skybolt::sim::World* mWorld;

private:
	const std::vector<DefaultContextActionPtr> mContextActions;
	ScenarioObjectTypeMap mScenarioObjectTypes;
	TreeItemModel* mModel;
	QTreeView* mView;

	TreeItemPtr mRootItem;
	std::map<ScenarioObjectPtr, std::shared_ptr<ScenarioObjectTreeItem>> mItemsMap;
};
