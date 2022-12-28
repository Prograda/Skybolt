#include "TreeItemContextAction.h"
#include "WorldTreeWidget.h"

TreeItemContextActionAdapter::TreeItemContextActionAdapter(const DefaultContextActionPtr& wrapped) : mWrapped(wrapped) {}

std::string TreeItemContextActionAdapter::getName() const { return mWrapped->getName(); }

bool TreeItemContextActionAdapter::handles(const TreeItem& object) const
{
	auto item = dynamic_cast<const EntityTreeItem*>(&object);
	if (item)
	{
		ActionContext context;
		context.entity = item->data.lock().get();
		return mWrapped->handles(context);
	}
	return false;
}

void TreeItemContextActionAdapter::execute(TreeItem& object) const
{
	auto item = dynamic_cast<const EntityTreeItem*>(&object);
	if (item)
	{
		ActionContext context;
		context.entity = item->data.lock().get();
		mWrapped->execute(context);
	}
}
