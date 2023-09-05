/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioTreeWidget.h"
#include "Registry.h"
#include "TreeItems.h"
#include "Icon/SprocketIcons.h"
#include "QtUtil/QtTimerUtil.h"
#include "Scenario/EntityObjectType.h"
#include "Scenario/ScenarioObject.h"
#include "Scenario/ScenarioSelectionModel.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltEngine/Scenario/Scenario.h>

#include <QLayout>
#include <QMenu>
#include <QTreeView>

using namespace skybolt;
using namespace skybolt::sim;

struct ScenarioObjectRegistryListener : public RegistryListener<ScenarioObject>
{
	ScenarioObjectRegistryListener(ScenarioTreeWidget* scenarioTreeWidget, const std::type_index& sceneObjectType, const ScenarioObjectRegistryPtr& registry) :
		mScenarioTreeWidget(scenarioTreeWidget),
		mSceneObjectType(sceneObjectType),
		mRegistry(registry)
	{}

	void itemAdded(const ScenarioObjectPtr& object) override
	{
		mScenarioTreeWidget->addObjects(mSceneObjectType, {object});
	}

	void itemAboutToBeRemoved(const ScenarioObjectPtr& object) override
	{
		mScenarioTreeWidget->removeObjects(mSceneObjectType, {object});
	}

private:
	ScenarioTreeWidget* mScenarioTreeWidget;
	std::type_index mSceneObjectType;
	ScenarioObjectRegistryPtr mRegistry;
};

struct ScenarioObjectTreeItem : public SimpleTreeItem
{
	ScenarioObjectTreeItem(const ScenarioObjectPtr& object) :
		SimpleTreeItem(QString::fromStdString(object->getName()), object->getIcon()),
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

	mRootItem = std::make_shared<SimpleTreeItem>("", getSprocketIcon(SprocketIcon::Folder));
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

	for (const auto& [id, type] : config.scenarioObjectTypes)
	{
		addObjects(id, type->objectRegistry->getItems());

		auto listener = std::make_unique<ScenarioObjectRegistryListener>(this, id, type->objectRegistry);
		type->objectRegistry->addListener(listener.get());
		mRegistryListeners.push_back(std::move(listener));
	}

	mView->expandAll();

	createAndStartIntervalTimer(100, this, [this] {
		updateTreeItemParents();
	});
}

ScenarioTreeWidget::~ScenarioTreeWidget() = default;

bool ScenarioTreeWidget::shouldDisplayItem(const ScenarioObject& object) const
{
	return true;
}

std::optional<std::string> ScenarioTreeWidget::getParentFolderName(const ScenarioObject& object) const
{
	if (auto type = findOptional(mScenarioObjectTypes, std::type_index(typeid(object))).value_or(nullptr); type)
	{
		if (!type->category.empty())
		{
			return type->category;
		}
	}
	return std::nullopt;
}

void ScenarioTreeWidget::addObjects(const std::type_index& sceneObjectType, const std::set<ScenarioObjectPtr>& objects)
{
	std::vector<TreeItemPtr> children;
	for (const auto& object : objects)
	{
		if (shouldDisplayItem(*object))
		{
			auto item = std::make_shared<ScenarioObjectTreeItem>(object);
			mItemsMap[object] = item;
			children.push_back(item);
		}
	}
	mModel->addChildren(*mRootItem, children);
	updateTreeItemParents();
}

void ScenarioTreeWidget::removeObjects(const std::type_index& sceneObjectType, const std::set<ScenarioObjectPtr>& objects)
{
	for (const auto& object : objects)
	{
		if (auto i = mItemsMap.find(object); i != mItemsMap.end())
		{
			mModel->removeItem(*i->second);
			mItemsMap.erase(i);
		}
	}
	updateTreeItemParents();
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

TreeItemPtr ScenarioTreeWidget::getOrCreateFolder(const std::string& name)
{
	if (auto item = findOptional(mFoldersMap, name); item)
	{
		return *item;
	}

	QIcon folderIcon = getSprocketIcon(SprocketIcon::Folder);
	auto item = std::make_shared<SimpleTreeItem>(QString::fromStdString(name), folderIcon);
	mModel->addChildren(*mRootItem, { item });
	mFoldersMap[name] = item;
	return item;
}

TreeItemPtr ScenarioTreeWidget::getParent(const ScenarioObject& object)
{
	TreeItemPtr parent;
	if (ScenarioObjectPtr objectParent = object.getParent(); objectParent)
	{
		if (auto treeItem = findOptional(mItemsMap, objectParent); treeItem)
		{
			return *treeItem;
		}
	}

	if (auto category = getParentFolderName(object); category)
	{
		return getOrCreateFolder(*category);
	}

	return mRootItem;
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
