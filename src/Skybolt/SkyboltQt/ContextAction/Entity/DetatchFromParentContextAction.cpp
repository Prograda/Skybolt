/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DetatchFromParentContextAction.h"
#include <SkyboltSim/Components/AttacherComponent.h>

using namespace skybolt;
using namespace skybolt::sim;

bool DetatchFromParentContextAction::handles(const Entity& entity) const
{
	auto component = entity.getFirstComponent<AttacherComponent>();
	return  component && component->state;
}

void DetatchFromParentContextAction::execute(Entity& entity) const
{
	AttacherComponentPtr component = entity.getFirstComponent<AttacherComponent>();
	assert(component);
	component->state = std::nullopt;
}
