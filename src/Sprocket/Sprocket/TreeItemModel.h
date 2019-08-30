/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/World.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QAbstractItemModel>
#include <QIcon>

typedef std::shared_ptr<class TreeItem> TreeItemPtr;

class TreeItem
{
	friend class TreeItemModel;
public:
	TreeItem(const QIcon& icon) : mIcon(icon) {}
	virtual ~TreeItem() {}

	virtual const QString& getLabel() = 0;
	virtual const QIcon& getIcon() const { return mIcon; }

	//! Registry item interface
	std::string getName() { return getLabel().toStdString(); }

private:
	std::vector<TreeItemPtr> mChildren;
	TreeItem* mParent = nullptr;
	QIcon mIcon;
};

class TreeItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeItemModel(const TreeItemPtr& root, QObject *parent = 0);

	TreeItem* getTreeItem(const QModelIndex &index) const;

	void addChildren(TreeItem& item, const std::vector<TreeItemPtr>& children);
	void insertChildren(TreeItem& item, int position, const std::vector<TreeItemPtr>& children);
	void removeChildren(TreeItem& item, int position, int count);
	void clearChildren(TreeItem& item);

public:
	// QAbstractItemModel interface
	QVariant data(const QModelIndex &index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
	QModelIndex index(TreeItem* item) const;

private:
	TreeItemPtr mRootItem;
	std::set<TreeItem*> mItems;
};
