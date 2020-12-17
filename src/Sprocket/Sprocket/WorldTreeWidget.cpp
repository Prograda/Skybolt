/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WorldTreeWidget.h"
#include "IconFactory.h"

#include <SkyboltEngine/Scenario.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

using namespace skybolt;
using namespace skybolt::sim;

struct WorldTreeWidgetRegistryListener : RegistryListener<TreeItem>
{
	WorldTreeWidgetRegistryListener(WorldTreeWidget* worldTreeWidget, const TreeItemPtr& parent, const std::shared_ptr<Registry<TreeItem>>& registry) :
		mWorldTreeWidget(worldTreeWidget),
		mParent(parent),
		mRegistry(registry)
	{}

	void itemAdded(const TreeItemPtr& function) override
	{
		mWorldTreeWidget->setItemsUnderParent(*mParent, *mRegistry);
	}

	void itemRemoved(const std::string& name) override
	{
		mWorldTreeWidget->setItemsUnderParent(*mParent, *mRegistry);
	}

private:
	WorldTreeWidget* mWorldTreeWidget;
	TreeItemPtr mParent;
	std::shared_ptr<Registry<TreeItem>> mRegistry;
};

struct RootItem : public TreeItem
{
	RootItem(const QIcon& icon) : TreeItem(icon) {}
	const QString& getLabel() override
	{
		static QString empty;
		return empty;
	}
};

typedef std::function<bool(const QString&)> StringValidator;

QString modalNameInputDialog(const QString& defaultName, StringValidator validator)
{
	QDialog dialog(nullptr, Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
	dialog.setWindowTitle("Enter name");

	QVBoxLayout* layout = new QVBoxLayout();
	dialog.setLayout(layout);

	QLineEdit* text = new QLineEdit();
	layout->addWidget(text);

	QLabel* label = new QLabel();
	label->setStyleSheet("QLabel { color : red; }");
	layout->addWidget(label);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	layout->addWidget(buttonBox);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	QObject::connect(text, &QLineEdit::textChanged, [validator, buttonBox, label](const QString& str) {
		bool valid = validator(str);
		buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(valid);
		if (valid)
		{
			label->setText("");
		}
		else
		{
			label->setText("Name already used");
		}
	});
	text->setText(defaultName);

	if (dialog.exec() == QDialog::Accepted)
	{
		return text->text();
	}

	return "";
}

QString modalFunctionNameInputDialog(const TreeItemType& type)
{
	QString defaultName = QString::fromStdString(type.itemRegistry->createUniqueItemName(type.name));
	auto registry = type.itemRegistry;
	return modalNameInputDialog(defaultName, [registry](const QString& str) {
		return (registry->findByName(str.toStdString()) == nullptr);
	});
}

static void connectTreeItemTypeMenuAction(const TreeItemType& type, QAction* action, const std::string& subType = "")
{
	const TreeItemType* typePtr = &type;
	QObject::connect(action, &QAction::triggered, [=]()
	{
		QString name = modalFunctionNameInputDialog(*typePtr);
		if (!name.isEmpty())
		{
			typePtr->itemCreator(name.toStdString(), subType);
		}
	});
}

static QMenu* createCreateMenu(World* world, EntityFactory* factory, const std::vector<TreeItemType>& types)
{
	QMenu* menu = new QMenu();

	// Entity menu
	QMenu* entityMenu = new QMenu("Entity");
	menu->addMenu(entityMenu);

	for (const std::string& templateName : factory->getTemplateNames())
	{
		QAction* action = new QAction(QString::fromStdString(templateName));
		entityMenu->addAction(action);

		QObject::connect(action, &QAction::triggered, [=]()
		{
			world->addEntity(factory->createEntity(templateName));
		});
	}

	for (const TreeItemType& type : types)
	{
		if (type.subTypes.empty())
		{
			QAction* action = new QAction(QString::fromStdString(type.name));
			menu->addAction(action);
			connectTreeItemTypeMenuAction(type, action);
		}
		else
		{
			QMenu* subMenu = new QMenu(QString::fromStdString(type.name));
			menu->addMenu(subMenu);
			for (const auto& subType : type.subTypes)
			{
				QAction* action = new QAction(QString::fromStdString(subType));
				subMenu->addAction(action);
				connectTreeItemTypeMenuAction(type, action, subType);
			}
		}
	}

	return menu;
}

sim::Entity* getParent(const sim::Entity& entity)
{
	sim::ParentReferenceComponent* component = entity.getFirstComponent<sim::ParentReferenceComponent>().get();
	sim::Entity* parent = component ? component->getParent() : nullptr;
	assert(parent != &entity);
	return parent;
}

WorldTreeWidget::WorldTreeWidget(const WorldTreeWidgetConfig& config) :
	mWorld(config.world),
	mContextActions(config.contextActions),
	mItemTypes(config.itemTypes)
{
	assert(!mItemTypes.empty());
	setLayout(new QVBoxLayout);

	QToolBar* toolbar = new QToolBar();

	QToolButton* createButton = new QToolButton();
	createButton->setIcon(getDefaultIconFactory().createIcon(IconFactory::Icon::Add));
	toolbar->addWidget(createButton);

	QToolButton* deleteButton = new QToolButton();
	deleteButton->setIcon(getDefaultIconFactory().createIcon(IconFactory::Icon::Remove));
	deleteButton->setEnabled(false);
	toolbar->addWidget(deleteButton);

	QMenu* menu = createCreateMenu(config.world, config.factory, mItemTypes);

	createButton->setMenu(menu);
	createButton->setPopupMode(QToolButton::InstantPopup);

	TreeItemPtr rootItem(new RootItem(getDefaultIconFactory().createIcon(IconFactory::Icon::Folder)));
	mModel = new TreeItemModel(rootItem, this);
	QTreeView* view = new QTreeView;
	view->setModel(mModel);
	view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(view, &QTreeView::customContextMenuRequested, this, [=](const QPoint& point) {
		QModelIndex index = view->indexAt(point);
		TreeItem* item = mModel->getTreeItem(index);
		if (item)
		{
			showContextMenu(*item, view->mapToGlobal(point));
		}
	});

	layout()->addWidget(toolbar);
	layout()->addWidget(view);

	QObject::connect(view->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex& current, const QModelIndex& previous)
	{
		const TreeItem* item = mModel->getTreeItem(current);
		if (item)
		{
			deleteButton->setEnabled(isDeletable(*item));
			emit selectionChanged(*item);
		}
	});

	QObject::connect(view, &QTreeView::clicked, [=](const QModelIndex& current)
	{
		const TreeItem* item = mModel->getTreeItem(current);
		if (item)
		{
			emit itemClicked(*item);
		}
	});

	QObject::connect(deleteButton, &QToolButton::pressed, [=]()
	{
		QModelIndex index = view->selectionModel()->currentIndex();
		if (index.isValid())
		{
			TreeItem* baseItem = mModel->getTreeItem(index);
			if (EntityTreeItem* item = dynamic_cast<EntityTreeItem*>(baseItem))
			{
				mWorld->removeEntity(item->data);
			}
			else
			{
				const TreeItemType* type = findItemType(*baseItem);
				if (type)
				{
					try
					{
						type->itemDeleter(baseItem);
					}
					catch (const std::exception& e)
					{
						QMessageBox::information(this, "", e.what());
					}
				}
			}
		}
	});

	QIcon folderIcon = getDefaultIconFactory().createIcon(IconFactory::Icon::Folder);

	mWorld->addListener(this);

	TreeItemPtr scenarioItem(new ScenarioTreeItem(folderIcon, "Scenario", config.scenario));
	mModel->addChildren(*rootItem, { scenarioItem });

	for (const TreeItemType& type : config.itemTypes)
	{
		auto item = std::make_shared<SimpleTreeItem>(folderIcon, QString::fromStdString(type.name), config.scenario);
		mTypeRootItems.push_back(item);
		mModel->addChildren(*scenarioItem, { item });
		setItemsUnderParent(*item, *type.itemRegistry);

		auto listener = std::make_unique<WorldTreeWidgetRegistryListener>(this, item, type.itemRegistry);
		type.itemRegistry->addListener(listener.get());
		mRegistryListeners.push_back(std::move(listener));
	}

	mEntityRootItem = std::make_shared<SimpleTreeItem>(folderIcon, "Entities", config.scenario);
	mModel->addChildren(*scenarioItem, { mEntityRootItem });

	updateEntityItems();
	view->expandAll();
}

WorldTreeWidget::~WorldTreeWidget()
{
	int i = 0;
	for (const TreeItemType& type : mItemTypes)
	{
		type.itemRegistry->removeListener(mRegistryListeners[i].get());
		++i;
	}
	mWorld->removeListener(this);

	for (const sim::EntityPtr& entity : mWorld->getEntities())
	{
		entity->removeListener(this);
	}
}

void WorldTreeWidget::update()
{
	if (mDirty)
	{
		updateEntityItems();
		mDirty = false;
	}
}

std::shared_ptr<EntityTreeItem> createEntityTreeItem(sim::Entity* entity)
{
	std::string name = getName(*entity);
	if (!name.empty())
	{
		QIcon nodeIcon = getDefaultIconFactory().createIcon(IconFactory::Icon::Node);
		return std::make_shared<EntityTreeItem>(nodeIcon, QString::fromStdString(name), entity);
	}
	return nullptr;
}

void WorldTreeWidget::updateEntityItems()
{
	mModel->clearChildren(*mEntityRootItem);

	std::map<sim::Entity*, std::vector<sim::Entity*>> itemChildren;
	std::map<sim::Entity*, TreeItemPtr> items;
	items[nullptr] = mEntityRootItem;

	// Add entities
	{
		const sim::World::Entities& entities = mWorld->getEntities();
		for (const sim::EntityPtr& entity : entities)
		{
			auto item = createEntityTreeItem(entity.get());
			if (item)
			{
				sim::Entity* parent = getParent(*entity);
				itemChildren[parent].push_back(entity.get());
				items[entity.get()] = item;
			}
		}

		for (const auto v : itemChildren)
		{
			auto parentIt = items.find(v.first);
			if (parentIt != items.end())
			{
				std::vector<TreeItemPtr> children;
				for (Entity* child : v.second)
				{
					children.push_back(items[child]);
				}

				mModel->addChildren(*parentIt->second, children);
			}
		}
	}
}

template <typename T>
std::vector<T> toVector(const std::set<T>& s)
{
	std::vector<T> r;
	r.insert(r.end(), s.begin(), s.end());
	return r;
}

void WorldTreeWidget::setItemsUnderParent(TreeItem& parent, const Registry<TreeItem>& registry)
{
	mModel->clearChildren(parent);
	mModel->addChildren(parent, toVector(registry.getItems()));
}

void WorldTreeWidget::entityAdded(const sim::EntityPtr& entity)
{
	entity->addListener(this);
	mDirty = true;
}

void WorldTreeWidget::entityRemoved(const sim::EntityPtr& entity)
{
	entity->removeListener(this);
	mDirty = true;
}

void WorldTreeWidget::onComponentAdded(Entity* entity, Component* component)
{
	if (dynamic_cast<ParentReferenceComponent*>(component))
	{
		mDirty = true;
	}
}

void WorldTreeWidget::onComponentRemove(Entity* entity, Component* component)
{
	if (dynamic_cast<ParentReferenceComponent*>(component))
	{
		mDirty = true;
	}
}

const TreeItemType* WorldTreeWidget::findItemType(const TreeItem& item) const
{
	const std::type_info& id = typeid(item);
	for (const TreeItemType& type : mItemTypes)
	{
		if (type.itemTypeId == id)
		{
			return &type;
		}
	}
	return nullptr;
}

bool WorldTreeWidget::isDeletable(const TreeItem& item) const
{
	if (const EntityTreeItem* entityTreeItem = dynamic_cast<const EntityTreeItem*>(&item))
	{
		return !entityTreeItem->data->getFirstComponent<sim::ProceduralLifetimeComponent>().get();
	}

	return true;
}

void WorldTreeWidget::showContextMenu(TreeItem& item, const QPoint& point)
{
	QMenu menu;

	for (const TreeItemContextActionPtr& action : mContextActions)
	{
		if (action->handles(item))
		{
			TreeItem* itemPtr = &item;
			menu.addAction(QString::fromStdString(action->getName()), [action, itemPtr]() { action->execute(*itemPtr); });
		}
	}

	if (!menu.actions().isEmpty())
	{
		menu.exec(point);
	}
}
