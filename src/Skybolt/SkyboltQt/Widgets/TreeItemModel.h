/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"

#include <SkyboltSim/World.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QAbstractItemModel>
#include <QIcon>

class TreeItem : public QObject
{
	Q_OBJECT
	friend class TreeItemModel;
public:
	TreeItem(const QIcon& icon) : mIcon(icon) {}
	virtual ~TreeItem() {}

	virtual const QString& getLabel() const = 0;
	virtual void setLabel(const QString& label) = 0;

	virtual const QIcon& getIcon() const { return mIcon; }

public:
	Q_SIGNAL void labelChanged(const QString& newLabel);

private:
	std::vector<TreeItemPtr> mChildren;
	TreeItem* mParent = nullptr;
	QIcon mIcon;
};

class SimpleTreeItem : public TreeItem
{
public:
	SimpleTreeItem(const QString& label, const QIcon& icon) : TreeItem(icon), mLabel(label) {}
	~SimpleTreeItem() override = default;

	const QString& getLabel() const override { return mLabel; }
	void setLabel(const QString& label) override;

private:
	QString mLabel;
};

class TreeItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeItemModel(const TreeItemPtr& root, QObject *parent = 0);

	TreeItemPtr getTreeItem(const QModelIndex &index) const;
	QModelIndex index(TreeItem* item) const;

	void addChildren(TreeItem& item, const std::vector<TreeItemPtr>& children);
	void insertChildren(TreeItem& item, int position, const std::vector<TreeItemPtr>& children);
	void removeChildren(TreeItem& item, int position, int count);
	void removeChild(TreeItem& item, const TreeItem& child);
	void clearChildren(TreeItem& item);
	const std::vector<TreeItemPtr>& getChildren(TreeItem& item) const;
	TreeItemPtr findChildByLabel(const TreeItem& item, const QString& label) const; //!< @returns nullptr if not found

	void removeItem(TreeItem& item);

	//! @returns -1 if child is not a child of item
	int getChildPosition(const TreeItem& item, const TreeItem& child);

	TreeItem* getParent(const TreeItem& item) const { return item.mParent; }

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
	TreeItemPtr mRootItem;
	std::set<TreeItemPtr> mItems;
};
