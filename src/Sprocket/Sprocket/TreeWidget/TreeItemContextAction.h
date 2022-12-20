/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TreeItemModel.h"
#include "Sprocket/ContextAction/ActionContext.h"

typedef std::shared_ptr<ContextAction<TreeItem>> TreeItemContextActionPtr;

class TreeItemContextActionAdapter : public ContextAction<TreeItem>
{
public:
	TreeItemContextActionAdapter(const DefaultContextActionPtr& wrapped);

	std::string getName() const override;

	bool handles(const TreeItem& object) const override;

	void execute(TreeItem& object) const override;

private:
	DefaultContextActionPtr mWrapped;
};

inline TreeItemContextActionPtr adaptToTreeItem(const DefaultContextActionPtr& action)
{
	return std::make_shared<TreeItemContextActionAdapter>(action);
}

inline std::vector<TreeItemContextActionPtr> adaptToTreeItems(const std::vector<DefaultContextActionPtr>& actions)
{
	std::vector<TreeItemContextActionPtr> r;
	for (auto a : actions)
	{
		r.push_back(adaptToTreeItem(a));
	}
	return r;
}
