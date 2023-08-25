/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectPicker.h"

#include "PlanetPointPicker.h"
#include "ScreenTransformUtil.h"
#include "Scenario/EntityObjectType.h"

#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

ScenarioObjectPicker createScenarioObjectPicker(const ScenarioObjectTypeMap& entityObjectTypes)
{
	return [entityObjectTypes](const sim::Vector3& viewPosition, const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, const ScenarioObjectPredicate& predicate) -> std::optional<PickedScenarioObject> {

		const glm::dmat4 invViewProjTransform = glm::inverse(viewProjTransform);
		sim::Vector3 viewDirection = screenToWorldDirection(viewPosition, invViewProjTransform, pointNdc);

		std::optional<PickedScenarioObject> pickedObject;
		double pickedEntityDistance = std::numeric_limits<double>::infinity();

		for (const auto& [typeIndex, type] : entityObjectTypes)
		{
			for (const auto& object : type->objectRegistry->getItems())
			{
				if (!predicate || predicate(*object))
				{
					if (auto position = object->intersectRay(viewPosition, viewDirection, viewProjTransform); position)
					{
						double distance = glm::distance(viewPosition, *position);
						if (distance < pickedEntityDistance)
						{
							pickedObject = PickedScenarioObject({
								object,
								*position
								});
							pickedEntityDistance = distance;
						}
					}
				}
			}
		}
		return pickedObject;
	};
}