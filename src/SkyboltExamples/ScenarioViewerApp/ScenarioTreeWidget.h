#pragma once

#include <SkyboltWidgets/SkyboltWidgetsFwd.h>
#include <SkyboltWidgets/Tree/TreeItemModel.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <QWidget>

class QTreeView;

struct ScenarioTreeWidgetConfig
{
	skybolt::sim::World* world;
};

struct EntityTreeItem;

class ScenarioTreeWidget : public QWidget
{
	Q_OBJECT

	friend struct ScenarioObjectRegistryListener;

public:
	ScenarioTreeWidget(const ScenarioTreeWidgetConfig& config);
	~ScenarioTreeWidget();

	Q_SIGNAL void entitySelected(const skybolt::sim::EntityId&);
	Q_SIGNAL void scenarioSelected();

private:
	void addOrRemoveEntities();
	void addEntities(const std::set<skybolt::sim::EntityPtr>& entities);
	void removeEntities(const std::set<skybolt::sim::EntityId>& entities);

	skybolt::TreeItemPtr getParent(const skybolt::sim::Entity& entity); //!< Never returns null
	void updateTreeItemParents();

	std::vector<skybolt::TreeItem*> getCurrentSelection() const;
	void setCurrentSelection(const std::vector<skybolt::TreeItem*>& items);

protected:
	skybolt::sim::World* mWorld;

private:
	skybolt::TreeItemModel* mModel;
	QTreeView* mView;

	skybolt::TreeItemPtr mRootItem;
	skybolt::TreeItemPtr mScenarioItem;
	std::map<skybolt::sim::EntityId, std::shared_ptr<EntityTreeItem>> mItemsMap;
};
