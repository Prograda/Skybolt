/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachToParentContextAction.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include "SkyboltQt/QtUtil/QtLayoutUtil.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/AttacherComponent.h>
#include <SkyboltSim/Components/AttachmentPointsComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QGridLayout>

using namespace skybolt;
using namespace skybolt::sim;

bool AttachToParentContextAction::handles(const Entity& entity) const
{
	return entity.getFirstComponent<AttacherComponent>() != nullptr;
}

static std::map<QString, EntityId> createEntityNameToIdMap(const sim::World& world, const std::string& ignoreEntity = "")
{
	std::map<QString, EntityId> candidateParentIds;
	for (const sim::EntityPtr& candidateParent : world.getEntities())
	{
		std::string name = sim::getName(*candidateParent);
		if (!name.empty() && name != ignoreEntity)
		{
			QString qtName = QString::fromStdString(name);
			candidateParentIds[qtName] = candidateParent->getId();
		}
	}
	return candidateParentIds;
}

static QComboBox* createEntityChooserComboBox(const std::map<QString, EntityId>& entities, QWidget* parent = nullptr)
{
	QComboBox* comboBox = new QComboBox(parent);
	for (const auto [name, id] : entities)
	{
		comboBox->addItem(name);
	}
	return comboBox;
}

static QStringList getEntityAttachmentPointAsQStringList(const sim::Entity& entity)
{
	QStringList items;
	if (auto points = entity.getFirstComponent<AttachmentPointsComponent>(); points)
	{
		for (const auto& [name, point] : points->attachmentPoints)
		{
			items.push_back(QString::fromStdString(name));
		}
	}
	return items;
}

void AttachToParentContextAction::execute(Entity& entity) const
{
	auto widget = new QWidget();
	auto layout = new QGridLayout(widget);
	
	std::map<QString, EntityId> entityNameToIdMap = createEntityNameToIdMap(*mWorld, sim::getName(entity));
	auto entityChooserComboBox = createEntityChooserComboBox(entityNameToIdMap, widget);
	addWidgetWithLabel(*layout, entityChooserComboBox, "Parent entity");

	auto parentAttachmentPointChooserComboBox = new QComboBox(widget);
	addWidgetWithLabel(*layout, parentAttachmentPointChooserComboBox, "Parent attachment point");

	auto ownAttachmentPointChooserComboBox = new QComboBox(widget);
	ownAttachmentPointChooserComboBox->addItem("");
	ownAttachmentPointChooserComboBox->addItems(getEntityAttachmentPointAsQStringList(entity));
	addWidgetWithLabel(*layout, ownAttachmentPointChooserComboBox, "Own attachment point");

	QObject::connect(entityChooserComboBox, &QComboBox::currentTextChanged, [this, entityNameToIdMap, parentAttachmentPointChooserComboBox] (const QString& entityName) {
		auto i = entityNameToIdMap.find(entityName);
		if (i != entityNameToIdMap.end())
		{
			if (auto parentEntity = mWorld->getEntityById(i->second); parentEntity)
			{
				parentAttachmentPointChooserComboBox->clear();
				parentAttachmentPointChooserComboBox->addItem("");
				parentAttachmentPointChooserComboBox->addItems(getEntityAttachmentPointAsQStringList(*parentEntity));
			}
		}
		});

	auto dialog = createDialogModal(widget, "Choose Entity");

	if (dialog->exec() == QDialog::Accepted)
	{
		auto i = entityNameToIdMap.find(entityChooserComboBox->currentText());
		if (i != entityNameToIdMap.end())
		{
			auto parentEntityId = i->second;
			if (const sim::AttacherComponentPtr& attachment = entity.getFirstComponent<AttacherComponent>(); attachment)
			{
				attachment->state = AttachmentState();
				attachment->state->parentEntityId = parentEntityId;
				attachment->state->parentEntityAttachmentPoint = parentAttachmentPointChooserComboBox->currentText().toStdString();
				attachment->state->ownEntityAttachmentPoint = ownAttachmentPointChooserComboBox->currentText().toStdString();
			}
		}
	}
}
