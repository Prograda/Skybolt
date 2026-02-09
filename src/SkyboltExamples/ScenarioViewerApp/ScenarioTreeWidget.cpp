/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioTreeWidget.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngineQt/Icon/SkyboltIcons.h>
#include <SkyboltSim/Components/AttacherComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltWidgets/Tree/TreeItemModel.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>

#include <QLayout>
#include <QMenu>
#include <QTreeView>

using namespace skybolt;
using namespace skybolt::sim;

struct EntityTreeItem : public SimpleTreeItem
{
	EntityTreeItem(const QString& name, const EntityId& entityId) :
		SimpleTreeItem(name, getSkyboltIcon(SkyboltIcon::Node)),
		entityId(entityId)
	{}

	EntityId entityId;
};

ScenarioTreeWidget::ScenarioTreeWidget(const ScenarioTreeWidgetConfig& config) :
	mWorld(config.world)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);

	mRootItem = std::make_shared<SimpleTreeItem>("", QIcon());
	mModel = new TreeItemModel(mRootItem, this);
	mView = new QTreeView(this);
	mView->setModel(mModel);
	mView->setContextMenuPolicy(Qt::CustomContextMenu);
	mView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	layout()->addWidget(mView);

	mScenarioItem = std::make_shared<SimpleTreeItem>("Scenario", getSkyboltIcon(SkyboltIcon::Node));
	mModel->addChildren(*mRootItem, {mScenarioItem});

	QObject::connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
		skybolt::sim::EntityId entity = skybolt::sim::nullEntityId();
		for (const QModelIndex& index : mView->selectionModel()->selection().indexes())
		{
			if (const TreeItemPtr& item = mModel->getTreeItem(index); item)
			{
				if (auto entityTreeItem = dynamic_cast<const EntityTreeItem*>(item.get()); entityTreeItem)
				{
					entity = entityTreeItem->entityId;
					break;
				}
				else if ( item == mScenarioItem)
				{
					Q_EMIT scenarioSelected();
				}
			}
		}
		Q_EMIT entitySelected(entity);
	});

	addOrRemoveEntities();
	updateTreeItemParents();

	mView->expandAll();

	createAndStartIntervalTimer(100, this, [this] {
		addOrRemoveEntities();
		updateTreeItemParents();
	});
}

ScenarioTreeWidget::~ScenarioTreeWidget() = default;

void ScenarioTreeWidget::addOrRemoveEntities()
{
	std::set<EntityPtr> objectsToAdd;
	std::set<EntityId> objectsToRemove;
	{
		// Make a set of all the items to display in the widget, and add any new items to objectsToAdd
		std::set<EntityId> includeObjects;
		for (const auto& entity : mWorld->getEntities())
		{
			if (getName(*entity).empty())
			{
				continue;
			}

			EntityId id = entity->getId();
			includeObjects.insert(id);
			if (mItemsMap.find(id) == mItemsMap.end())
			{
				objectsToAdd.insert(entity);
			}
		}

		// Make a set of objects to remove from the widget
		for (const auto& [entityId, treeitem] : mItemsMap)
		{
			if (includeObjects.find(entityId) == includeObjects.end())
			{
				objectsToRemove.insert(entityId);
			}
		}
	}

	removeEntities(objectsToRemove);
	addEntities(objectsToAdd);
}

void ScenarioTreeWidget::addEntities(const std::set<EntityPtr>& entities)
{
	std::vector<TreeItemPtr> children;
	for (const auto& entity : entities)
	{
		QString name = QString::fromStdString(getName(*entity));
		auto item = std::make_shared<EntityTreeItem>(name, entity->getId());
		mItemsMap[entity->getId()] = item;
		children.push_back(item);
	}
	mModel->addChildren(*mScenarioItem, children);
}

void ScenarioTreeWidget::removeEntities(const std::set<EntityId>& entities)
{
	for (const auto& entity : entities)
	{
		if (auto i = mItemsMap.find(entity); i != mItemsMap.end())
		{
			mModel->removeItem(*i->second);
			mItemsMap.erase(i);
		}
	}
}

std::vector<TreeItem*> ScenarioTreeWidget::getCurrentSelection() const
{
	QModelIndexList selected = mView->selectionModel()->selectedIndexes();
	std::vector<TreeItem*> r;
	for (auto i : selected)
	{
		if (TreeItemPtr item = mModel->getTreeItem(i); item)
		{
			r.push_back(item.get());
		}
	}
	return r;
}

void ScenarioTreeWidget::setCurrentSelection(const std::vector<TreeItem*>& items)
{
	if (getCurrentSelection() != items)
	{
		QItemSelection selected;
		for (const auto i : items)
		{
			selected.push_back(QItemSelectionRange(mModel->index(i)));
		}

		mView->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
	}
}

TreeItemPtr ScenarioTreeWidget::getParent(const Entity& entity)
{
	auto attacherComponent = entity.getFirstComponent<AttacherComponent>();
	if (!attacherComponent)
	{
		return nullptr;
	}
	if (auto i = mItemsMap.find(attacherComponent->parentEntityId); i != mItemsMap.end())
	{
		return i->second;
	}
	return nullptr;
}

void ScenarioTreeWidget::updateTreeItemParents()
{
	std::vector<TreeItem*> selection = getCurrentSelection();

	for (const auto& [entityId, item] : mItemsMap)
	{
		auto entity = mWorld->getEntityById(entityId);
		if (!entity)
		{
			continue;
		}

		if (TreeItemPtr newParent = getParent(*entity); newParent)
		{
			TreeItem* currentParent = mModel->getParent(*item);
			if (currentParent != newParent.get())
			{
				mModel->removeChild(*currentParent, *item);
				mModel->addChildren(*newParent, {item});
				mView->expand(mModel->index(newParent.get()));
			}
		}
	}

	setCurrentSelection(selection); // Selection can change after removing item, so we need to restore it here.
}
