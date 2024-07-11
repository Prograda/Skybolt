/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Entity.h"

namespace skybolt {
namespace sim {

template <class ComponentT>
sim::EntityPtr findNearestEntityWithComponent(const std::vector<EntityPtr>& entities, const sim::Vector3& position)
{
	sim::EntityPtr result = nullptr;
	double resultDistanceSq = 0;
	for (const sim::EntityPtr& entity : entities)
	{
		if (auto* component = entity->getFirstComponent<ComponentT>().get())
		{
			const auto& entityPosition = getPosition(*entity);
			if (entityPosition)
			{
				sim::Vector3 diff = position - *entityPosition;
				double distanceSq = glm::dot(diff, diff);
				if (!result || distanceSq < resultDistanceSq)
				{
					result = entity;
					resultDistanceSq = distanceSq;
				}
			}
		}
	}
	return result;
}

} // namespace sim
} // namespace skybolt