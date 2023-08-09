/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "Sprocket/SprocketFwd.h"

#include <QString>

template <class T>
struct TreeItemT : public TreeItem
{
	QString label;
	T data;
	
	TreeItemT(const QIcon& icon, const QString& label, const T& data)
		: TreeItem(icon), label(label), data(data)
	{
	}

	const QString& getLabel() const override
	{
		return label;
	}
};

class SimpleTreeItem : public TreeItem
{
public:
	SimpleTreeItem(const QString& label, const QIcon& icon) : TreeItem(icon), mLabel(label) {}
	~SimpleTreeItem() override = default;

	const QString& getLabel() const override { return mLabel; }

private:
	QString mLabel;
};
