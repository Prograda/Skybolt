/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachToParentContextAction.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <QComboBox>

using namespace skybolt;
using namespace skybolt::sim;

bool AttachToParentContextAction::handles(const Entity& entity) const
{
	return true;
}

void AttachToParentContextAction::execute(Entity& entity) const
{
	QComboBox* comboBox = new QComboBox();
	std::map<QString, EntityId> candidateParentIds;
	for (const sim::EntityPtr& candidateParent : mWorld->getEntities())
	{
		std::string name = sim::getName(*candidateParent);
		if (!name.empty() && name != sim::getName(entity))
		{
			QString qtName = QString::fromStdString(name);
			candidateParentIds[qtName] = candidateParent->getId();
			comboBox->addItem(qtName);
		}
	}

	auto dialog = createDialogModal(comboBox, "Choose Entity");

	if (dialog->exec() == QDialog::Accepted)
	{
		auto i = candidateParentIds.find(comboBox->currentText());
		if (i != candidateParentIds.end())
		{
			auto parentEntityId = i->second;
			sim::AttachmentComponentPtr attachment = entity.getFirstComponent<AttachmentComponent>();
			if (!attachment)
			{
				AttachmentParams params;
				params.positionRelBody = math::vec3Zero();
				params.orientationRelBody = math::quatIdentity();
				attachment = std::make_shared<AttachmentComponent>(params, mWorld, &entity);
				entity.addComponent(attachment);
			}

			attachment->setParentEntityId(parentEntityId);
		}
	}
}
