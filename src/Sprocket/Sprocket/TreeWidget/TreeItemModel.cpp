/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TreeItemModel.h"
#include <SkyboltSim/World.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QAbstractItemModel>

TreeItemModel::TreeItemModel(const TreeItemPtr& root, QObject *parent)
	: QAbstractItemModel(parent),
	mRootItem(root)
{
}

TreeItem* TreeItemModel::getTreeItem(const QModelIndex &index) const
{
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (mItems.find(item) != mItems.end())
		return item;
	return nullptr;
}

void TreeItemModel::addChildren(TreeItem& item, const std::vector<TreeItemPtr>& children)
{
	insertChildren(item, item.mChildren.size(), children);
}

void TreeItemModel::insertChildren(TreeItem& item, int position, const std::vector<TreeItemPtr>& children)
{
	if (children.empty())
		return;

	for (const TreeItemPtr& item : children)
	{
		mItems.insert(item.get());
	}

	beginInsertRows(index(&item), position, position + (int)children.size() - 1);
	item.mChildren.insert(item.mChildren.begin() + position, children.begin(), children.end());

	for (const TreeItemPtr& child : children)
	{
		child->mParent = &item;
	}

	endInsertRows();
}

void TreeItemModel::removeChildren(TreeItem& item, int position, int count)
{
	if (count == 0)
		return;

	beginRemoveRows(index(&item), position, position + count);

	for (int i = position; i < position + count; ++i)
	{
		TreeItem* child = item.mChildren[i].get();
		child->mParent = nullptr;
		mItems.erase(child);
	}

	item.mChildren.erase(item.mChildren.begin() + position, item.mChildren.begin() + position + count);
	endRemoveRows();
}

void TreeItemModel::removeChild(TreeItem& item, const TreeItem& child)
{
	if (int i = getChildPosition(item, child); i >= 0)
	{
		removeChildren(item, i, 1);
	}
}

void TreeItemModel::clearChildren(TreeItem& item)
{
	removeChildren(item, 0, (int)item.mChildren.size());
}

int TreeItemModel::getChildPosition(const TreeItem& item, const TreeItem& child)
{
	int i = 0;
	for (const auto& c : item.mChildren)
	{
		if (c.get() == &child)
		{
			return i;
		}
		++i;
	}
	return -1;
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem* parentItem;

	if (!parent.isValid())
		parentItem = mRootItem.get();
	else
		parentItem = getTreeItem(parent);

	if (!parentItem)
		return QModelIndex();

	TreeItem* childItem = parentItem->mChildren.at(row).get();
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TreeItemModel::index(TreeItem* item) const
{
	int row = 0;
	if (item->mParent)
	{
		for (const TreeItemPtr& child : item->mParent->mChildren)
		{
			if (child.get() == item)
			{
				break;
			}
			++row;
		}
	}

	return createIndex(row, 0, item);
}

QModelIndex TreeItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = getTreeItem(index);

	if (childItem == nullptr || childItem->mParent == nullptr || childItem->mParent->mParent == nullptr)
		return QModelIndex();

	int row = 0;
	for (const TreeItemPtr& item : childItem->mParent->mParent->mChildren)
	{
		if (item.get() == childItem->mParent)
		{
			break;
		}
		++row;
	}

	return createIndex(row, 0, childItem->mParent);
}

int TreeItemModel::rowCount(const QModelIndex &itemIndex) const
{
	TreeItem* item;
	if (itemIndex.column() > 0)
		return 0;

	if (!itemIndex.isValid())
		item = mRootItem.get();
	else
		item = getTreeItem(itemIndex);

	if (!item)
		return 0;

	return (int)item->mChildren.size();
}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeItem *item = getTreeItem(index);

	if (!item)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		return item->getLabel();
	}
	else if (role == Qt::DecorationRole)
	{
		return item->getIcon();
	}

	return QVariant();
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemFlags();

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}
