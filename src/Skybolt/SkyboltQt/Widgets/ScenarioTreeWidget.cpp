/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioTreeWidget.h"
#include "TreeItemModel.h"
#include "Icon/SkyboltIcons.h"
#include "QtUtil/QtTimerUtil.h"
#include "Scenario/EntityObjectType.h"
#include "Scenario/Registry.h"
#include "Scenario/ScenarioObject.h"
#include "Scenario/ScenarioSelectionModel.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/Scenario/Scenario.h>

#include <QLayout>
#include <QMenu>
#include <QTreeView>

using namespace skybolt;
using namespace skybolt::sim;

struct ScenarioObjectTreeItem : public SimpleTreeItem
{
	ScenarioObjectTreeItem(const ScenarioObjectPtr& object) :
		SimpleTreeItem(QString::fromStdString(object->getDisplayName()), object->getIcon()),
		object(object)
	{}

	ScenarioObjectPtr object;
};

ScenarioTreeWidget::ScenarioTreeWidget(const ScenarioTreeWidgetConfig& config) :
	mWorld(config.world),
	mContextActions(config.contextActions),
	mScenarioObjectTypes(config.scenarioObjectTypes)
{
	setLayout(new QVBoxLayout(this));

	mRootItem = std::make_shared<SimpleTreeItem>("", getSkyboltIcon(SkyboltIcon::Folder));
	mModel = new TreeItemModel(mRootItem, this);
	mView = new QTreeView(this);
	mView->setModel(mModel);
	mView->setContextMenuPolicy(Qt::CustomContextMenu);
	mView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	connect(mView, &QTreeView::customContextMenuRequested, this, [=](const QPoint& point) {
		QModelIndex index = mView->indexAt(point);
		TreeItemPtr item = mModel->getTreeItem(index);
		if (item)
		{
			showContextMenu(*item, mView->mapToGlobal(point));
		}
	});

	layout()->addWidget(mView);

	QObject::connect(config.selectionModel, &ScenarioSelectionModel::selectionChanged, [this]
		(const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected)
	{
		std::vector<TreeItem*> selection;
		for (auto object : selected)
		{
			if (auto item = findOptional(mItemsMap, object); item)
			{
				selection.push_back(item->get());
			}
		};
		setCurrentSelection(selection);
	});

	QObject::connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged, [this, selectionModel = config.selectionModel](const QItemSelection& selected, const QItemSelection& deselected) {
		SelectedScenarioObjects objects;
		for (const QModelIndex& index : mView->selectionModel()->selection().indexes())
		{
			if (const TreeItemPtr& item = mModel->getTreeItem(index); item)
			{
				if (auto scenarioTreeItem = dynamic_cast<const ScenarioObjectTreeItem*>(item.get()); scenarioTreeItem)
				{
					objects.push_back(scenarioTreeItem->object);
				}
			}
		}
		selectionModel->setSelectedItems(objects);
	});

	addOrRemoveObjects();
	updateTreeItemParents();

	mView->expandAll();

	createAndStartIntervalTimer(100, this, [this] {
		addOrRemoveObjects();
		updateTreeItemParents();
		updateItemDisplayNames();
	});
}

ScenarioTreeWidget::~ScenarioTreeWidget() = default;

bool ScenarioTreeWidget::shouldDisplayItem(const ScenarioObject& object) const
{
	return true;
}

void ScenarioTreeWidget::addOrRemoveObjects()
{
	std::set<ScenarioObjectPtr> objectsToAdd;
	std::set<ScenarioObjectPtr> objectsToRemove;
	{
		// Make a set of all the items to display in the widget, and add any new items to objectsToAdd
		std::set<ScenarioObjectPtr> includeObjects;
		for (const auto& [id, type] : mScenarioObjectTypes)
		{
			for (const ScenarioObjectPtr& object : type->objectRegistry->getItems())
			{
				if (shouldDisplayItem(*object))
				{
					includeObjects.insert(object);
					if (mItemsMap.find(object) == mItemsMap.end())
					{
						objectsToAdd.insert(object);
					}
				}
			}
		}

		// Make a set of objects to remove from the widget
		for (const auto& [object, treeitem] : mItemsMap)
		{
			if (includeObjects.find(object) == includeObjects.end())
			{
				objectsToRemove.insert(object);
			}
		}
	}

	removeObjects(objectsToRemove);
	addObjects(objectsToAdd);
}

void ScenarioTreeWidget::addObjects(const std::set<ScenarioObjectPtr>& objects)
{
	std::vector<TreeItemPtr> children;
	for (const auto& object : objects)
	{
		auto item = std::make_shared<ScenarioObjectTreeItem>(object);
		mItemsMap[object] = item;
		children.push_back(item);
	}
	mModel->addChildren(*mRootItem, children);
}

void ScenarioTreeWidget::removeObjects(const std::set<ScenarioObjectPtr>& objects)
{
	for (const auto& object : objects)
	{
		if (auto i = mItemsMap.find(object); i != mItemsMap.end())
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

TreeItemPtr ScenarioTreeWidget::createFolder(TreeItem& parent, const std::string& name)
{
	QIcon folderIcon = getSkyboltIcon(SkyboltIcon::Folder);
	auto item = std::make_shared<SimpleTreeItem>(QString::fromStdString(name), folderIcon);
	mModel->addChildren(parent, { item });
	return item;
}

TreeItemPtr ScenarioTreeWidget::getParent(const ScenarioObject& object)
{
	ScenarioObjectPath directory = object.getDirectory();

	TreeItemPtr parent = mRootItem;
	for (const std::string& folder : directory)
	{
		TreeItemPtr child = mModel->findChildByLabel(*parent, QString::fromStdString(folder));

		if (!child)
		{
			child = createFolder(*parent, folder);
		}
		parent = child;
	}
	return parent;
}

void ScenarioTreeWidget::updateTreeItemParents()
{
	std::vector<TreeItem*> selection = getCurrentSelection();

	for (const auto& [object, item] : mItemsMap)
	{
		if (TreeItemPtr newParent = getParent(*object); newParent)
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

void ScenarioTreeWidget::updateItemDisplayNames()
{
	for (const auto& [object, item] : mItemsMap)
	{
		const QString& displayName = QString::fromStdString(object->getDisplayName());
		if (item->getLabel() != displayName)
		{
			item->setLabel(displayName);
		}
	}
}

ScenarioObjectPtr ScenarioTreeWidget::findScenarioObject(const TreeItem& item) const
{
	if (auto scenarioItem = dynamic_cast<const ScenarioObjectTreeItem*>(&item); scenarioItem)
	{
		return scenarioItem->object;
	}
	return nullptr;
}

ActionContext ScenarioTreeWidget::toActionContext(const sim::World& world, const TreeItem& item) const
{
	ActionContext context;
	if (const ScenarioObjectPtr& scenarioObject = findScenarioObject(item); scenarioObject)
	{
		if (auto entityScenarioObject = dynamic_cast<EntityObject*>(scenarioObject.get()); entityScenarioObject)
		{
			context.entity = world.getEntityById(entityScenarioObject->data).get();
		}
	}
	return context;
}

void ScenarioTreeWidget::showContextMenu(TreeItem& item, const QPoint& point)
{
	QMenu menu;

	for (const DefaultContextActionPtr& action : mContextActions)
	{
		ActionContext context = toActionContext(*mWorld, item);
		if (action->handles(context))
		{
			menu.addAction(QString::fromStdString(action->getName()), [this, &item, action]() {
				// Confirm that the action is still supported before executing,
				// as the context might have changed since the action was created.
				ActionContext context = toActionContext(*mWorld, item);
				context.widget = this;
				if (action->handles(context))
				{
					action->execute(context);
				}
			});
		}
	}

	if (!menu.actions().isEmpty())
	{
		menu.exec(point);
	}
}
