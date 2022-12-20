/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachToParentContextAction.h"
#include "Sprocket/QDialogHelpers.h"
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>
#include <SkyboltSim/World.h>

#include <QComboBox>

using namespace skybolt;
using namespace skybolt::sim;

bool AttachToParentContextAction::handles(const Entity& entity) const
{
	auto templateNameComponent = entity.getFirstComponent<TemplateNameComponent>();
	if (templateNameComponent)
	{
		return getParentAttachment(entity) == nullptr;
	}
	return false;
}

void AttachToParentContextAction::execute(Entity& entity) const
{
	std::string templateName = entity.getFirstComponent<TemplateNameComponent>()->name;

	QComboBox* comboBox = new QComboBox();
	std::map<QString, AttachmentComponentPtr> candidateAttachments;
	for (const sim::EntityPtr& candidateParent : mWorld->getEntities())
	{
		auto components = candidateParent->getComponentsOfType<sim::AttachmentComponent>();
		for (auto component : components)
		{
			if (!component->getTarget() && component->getEntityTemplate() == templateName)
			{
				QString name = QString::fromStdString(sim::getName(*candidateParent));
				candidateAttachments[name] = component;
				comboBox->addItem(name);
			}
		}
	}

	auto dialog = createDialog(comboBox, "Choose Entity");

	if (dialog->exec() == QDialog::Accepted)
	{
		auto i = candidateAttachments.find(comboBox->currentText());
		if (i != candidateAttachments.end())
		{
			auto attachment = i->second;
			attachment->resetTarget(&entity);
		}
	}
}
