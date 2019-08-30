/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DetatchFromParentContextAction.h"
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>

using namespace skybolt;
using namespace skybolt::sim;

bool DetatchFromParentContextAction::handles(const Entity& entity) const
{
	auto templateNameComponent = entity.getFirstComponent<TemplateNameComponent>();
	if (templateNameComponent)
	{
		return getParentAttachment(entity) != nullptr;
	}
	return false;
}

void DetatchFromParentContextAction::execute(Entity& entity) const
{
	AttachmentComponentPtr attachment = getParentAttachment(entity);
	assert(attachment);
	attachment->resetTarget(nullptr);
}
