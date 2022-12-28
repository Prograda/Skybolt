/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "Sprocket/Registry.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>

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

	const QString& getLabel() override
	{
		return label;
	}
};

typedef TreeItemT<void*> SimpleTreeItem;
typedef TreeItemT<std::weak_ptr<skybolt::sim::Entity>> EntityTreeItem;
typedef TreeItemT<skybolt::Scenario*> ScenarioTreeItem;

struct TreeItemType
{
	TreeItemType(const std::type_index& itemTypeId) : itemTypeId(itemTypeId) {}

	std::string name;
	std::vector<std::string> subTypes; //!< Names of each sub type of this item. Leave empty if there's only one type.
	std::function<void(const std::string& name, const std::string& subType)> itemCreator;
	std::function<void(TreeItem*)> itemDeleter; //!< Throws exception if item is not disposable
	std::shared_ptr<Registry<TreeItem>> itemRegistry;
	std::type_index itemTypeId;
};