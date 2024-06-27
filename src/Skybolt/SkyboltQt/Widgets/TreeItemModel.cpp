/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TreeItemModel.h"
#include <SkyboltSim/World.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QAbstractItemModel>

void SimpleTreeItem::setLabel(const QString& label)
{
	mLabel = label;
	Q_EMIT labelChanged(label);
}

TreeItemModel::TreeItemModel(const TreeItemPtr& root, QObject *parent)
	: QAbstractItemModel(parent),
	mRootItem(root)
{
}

TreeItemPtr TreeItemModel::getTreeItem(const QModelIndex &index) const
{
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	auto i = std::find_if(mItems.begin(), mItems.end(), [&] (const TreeItemPtr& itemPtr) {
		return itemPtr.get() == item;
	});
	if (i != mItems.end())
		return *i;
	return nullptr;
}

void TreeItemModel::addChildren(TreeItem& item, const std::vector<TreeItemPtr>& children)
{
	insertChildren(item, int(item.mChildren.size()), children);
}

void TreeItemModel::insertChildren(TreeItem& item, int position, const std::vector<TreeItemPtr>& children)
{
	if (children.empty())
		return;

	for (const TreeItemPtr& child : children)
	{
		mItems.insert(child);
		connect(child.get(), &TreeItem::labelChanged, this, [this, child] {
			QModelIndex childIndex = index(child.get());
			dataChanged(childIndex, childIndex, { Qt::DisplayRole });
		});
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

	beginRemoveRows(index(&item), position, position + count - 1);

	for (int i = position; i < position + count; ++i)
	{
		TreeItemPtr child = item.mChildren[i];
		child->mParent = nullptr;
		mItems.erase(child);

		disconnect(child.get(), nullptr, nullptr, nullptr);
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

const std::vector<TreeItemPtr>& TreeItemModel::getChildren(TreeItem& item) const
{
	return item.mChildren;
}

TreeItemPtr TreeItemModel::findChildByLabel(const TreeItem& item, const QString& label) const
{
	const std::vector<TreeItemPtr>& children = item.mChildren;
	for (const auto& child : children)
	{
		if (child->getLabel() == label)
		{
			return child;
		}
	}
	return nullptr;
}

int TreeItemModel::getChildPosition(const TreeItem& item, const TreeItem& child)
{
	int i = -1;
	for (const auto& c : item.mChildren)
	{
		++i;
		if (c.get() == &child)
		{
			return i;
		}
	}
	return i;
}

void TreeItemModel::removeItem(TreeItem& item)
{
	if (TreeItem* parent = getParent(item); parent)
	{
		removeChild(*parent, item);
	}
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItemPtr parentItem;

	if (!parent.isValid())
		parentItem = mRootItem;
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
	else
	{
		// If item does not have a parent, it's the invisible root item which is represented by the null index.
		return QModelIndex();
	}

	return createIndex(row, 0, item);
}

QModelIndex TreeItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItemPtr childItem = getTreeItem(index);

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
	TreeItemPtr item;
	if (itemIndex.column() > 0)
		return 0;

	if (!itemIndex.isValid())
		item = mRootItem;
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

	TreeItemPtr item = getTreeItem(index);

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
