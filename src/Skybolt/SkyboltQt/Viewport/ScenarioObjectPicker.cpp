/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectPicker.h"

#include "PlanetPointPicker.h"
#include "ScreenTransformUtil.h"
#include "Scenario/EntityObjectType.h"

#include <SkyboltCommon/ContainerUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/NameComponent.h>

using namespace skybolt;

ScenarioObjectPicker createScenarioObjectPicker(const ScenarioObjectTypeMap& entityObjectTypes)
{
	return [entityObjectTypes](const sim::Vector3& viewPosition, const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, const ScenarioObjectPredicate& predicate) -> std::vector<PickedScenarioObject> {

		const glm::dmat4 invViewProjTransform = glm::inverse(viewProjTransform);
		sim::Vector3 viewDirection = screenToWorldDirection(viewPosition, invViewProjTransform, pointNdc);

		std::vector<std::pair<PickedScenarioObject, double>> pickedObjectDistances;

		for (const auto& [typeIndex, type] : entityObjectTypes)
		{
			for (const auto& object : type->objectRegistry->getItems())
			{
				if (!predicate || predicate(*object))
				{
					if (auto position = object->intersectRay(viewPosition, viewDirection, viewProjTransform); position)
					{
						double distance = glm::distance(viewPosition, *position);
						pickedObjectDistances.push_back({
							PickedScenarioObject({ object, *position }),
							distance
						});
					}
				}
			}
		}

		// Sort from near to far
		std::stable_sort(pickedObjectDistances.begin(), pickedObjectDistances.end(), [&](const auto& a, const auto& b) {
			return a.second < b.second;
			});

		// Return the objects
		std::vector<PickedScenarioObject> result;
		result.reserve(pickedObjectDistances.size());

		transform(pickedObjectDistances, result, [](const auto& i) { return i.first; });
		return result;
	};
}