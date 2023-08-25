/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScreenTransformUtil.h"
#include "PlanetPointPicker.h"

#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>

using namespace skybolt;

std::optional<PickedEntity> pickPointOnPlanet(const skybolt::sim::World& world, const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	sim::Vector3 dir = screenToWorldDirection(origin, invViewProjTransform, pointNdc);
	if (const sim::EntityPtr& entity = sim::findNearestEntityWithComponent<sim::PlanetComponent>(world.getEntities(), origin); entity)
	{
		if (auto position = getPosition(*entity); position)
		{
			auto component = entity->getFirstComponentRequired<sim::PlanetComponent>();
			if (auto pickedPosition = pickPointOnPlanet(*entity, *component, origin, dir); pickedPosition)
			{
				return PickedEntity({
					entity,
					*pickedPosition
				});
			}
		}
	}
	return std::nullopt;
}

std::optional<skybolt::sim::Vector3> pickPointOnPlanet(const sim::Entity& entity, const sim::PlanetComponent& planet, const sim::Vector3& viewPosition, const sim::Vector3& viewDirection)
{
	if (auto r = intersectRaySphere(viewPosition, viewDirection, *getPosition(entity), planet.radius); r)
	{
		return viewPosition + glm::inverse(*getOrientation(entity)) * (viewDirection * double(r->first));
	}
	return std::nullopt;
}