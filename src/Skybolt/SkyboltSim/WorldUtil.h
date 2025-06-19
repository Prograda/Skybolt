/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Entity.h"

namespace skybolt {
namespace sim {

//! @returns the nearest entity with a given component, or null if no entity found.
//! @param outComponent is the component found, or null if no entity/component found.
template <class ComponentT>
sim::EntityPtr findNearestEntityWithComponent(const std::vector<EntityPtr>& entities, const sim::Vector3& position, std::shared_ptr<ComponentT>& outComponent)
{
	sim::EntityPtr result = nullptr;
	double resultDistanceSq = 0;
	for (const sim::EntityPtr& entity : entities)
	{
		if (auto component = entity->getFirstComponent<ComponentT>(); component)
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
					outComponent = component;
				}
			}
		}
	}
	return result;
}

//! @returns the nearest entity with a given component, or null if no entity found.
template <class ComponentT>
sim::EntityPtr findNearestEntityWithComponent(const std::vector<EntityPtr>& entities, const sim::Vector3& position)
{
	std::shared_ptr<ComponentT> component;
	return findNearestEntityWithComponent(entities, position, component);
}

//! @returns the nearest entity with a given component, or null if no entity found.
//! @param outComponent is the component found, or null if no entity/component found.
template <class ComponentT>
std::vector<sim::EntityPtr> findEntitiesWithComponent(const std::vector<EntityPtr>& entities)
{
	std::vector<sim::EntityPtr> result;
	for (const sim::EntityPtr& entity : entities)
	{
		if (auto component = entity->getFirstComponent<ComponentT>(); component)
		{
			result.push_back(entity);
		}
	}
	return result;
}

} // namespace sim
} // namespace skybolt