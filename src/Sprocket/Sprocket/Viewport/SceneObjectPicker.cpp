/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SceneObjectPicker.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>

using namespace skybolt;

glm::dmat4 makeViewProjTransform(const sim::Vector3& origin, const sim::Quaternion& orientation, const sim::CameraState& camera, double aspectRatio)
{
	static glm::dquat orientationOffset =
		glm::angleAxis(math::halfPiD(), glm::dvec3(-1, 0, 0)) *
		glm::angleAxis(math::halfPiD(), glm::dvec3(0, -1, 0));

	glm::dmat4 m = glm::inverse(glm::translate(origin) * glm::dmat4(orientation * orientationOffset));
	return glm::infinitePerspective(double(camera.fovY), aspectRatio, camera.nearClipDistance) * m;
}

std::optional<sim::Vector3> worldToScreenNdcPoint(const glm::dmat4& viewProjTransform, const skybolt::sim::Vector3& pointWorldSpace)
{
	glm::dvec4 entityPoint = viewProjTransform * glm::dvec4(pointWorldSpace, 1.0);
	if (entityPoint.z > 0)
	{
		sim::Vector3 entityPointNdc(entityPoint.x / entityPoint.w, -entityPoint.y / entityPoint.w, entityPoint.z / entityPoint.w); // Note: y axis is flipped
		return entityPointNdc * 0.5 + sim::Vector3(0.5);
	}
	return std::nullopt;
}

skybolt::sim::Vector3 screenToWorldPoint(const glm::dmat4& invViewProjTransform, const sim::Vector3& pointNdc)
{
	glm::dvec4 point(pointNdc.x * 2.0 - 1.0, pointNdc.y * -2.0 + 1.0, pointNdc.z * 2.0 - 1.0, 1); // Note: y axis is flipped
	point = invViewProjTransform * point;
	point.x /= point.w;
	point.y /= point.w;
	point.z /= point.w;
	return point;
}

sim::Vector3 screenToWorldDirection(const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	sim::Vector3 point = screenToWorldPoint(invViewProjTransform, sim::Vector3(pointNdc.x, pointNdc.y, 0.99));
	return glm::normalize(point - origin);
}

std::optional<PickedSceneObject> pickPointOnPlanet(const skybolt::sim::World& world, const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	sim::Vector3 dir = screenToWorldDirection(origin, invViewProjTransform, pointNdc);
	if (const sim::EntityPtr& entity = sim::findNearestEntityWithComponent<sim::PlanetComponent>(world.getEntities(), origin); entity)
	{
		if (auto position = getPosition(*entity); position)
		{
			auto component = entity->getFirstComponentRequired<sim::PlanetComponent>();
			if (auto r = intersectRaySphere(origin, dir, *position, component->radius); r)
			{
				return PickedSceneObject({
					entity,
					origin + glm::inverse(*getOrientation(*entity)) * (dir * double(r->first))
				});
			}
		}
	}
	return std::nullopt;
}

SceneObjectPicker createSceneObjectPicker(const sim::World* world)
{
	return [world](const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, float pickRadiusNdc, const EntitySelectionPredicate& predicate) -> std::optional<PickedSceneObject> {

		std::optional<PickedSceneObject> pickedObject;
		float pickedEntityDistance = pickRadiusNdc;

		for (const auto& entity : world->getEntities())
		{
			if (!predicate || predicate(*entity))
			{
				if (auto position = getPosition(*entity); position)
				{
					if (auto entityPointNdc = worldToScreenNdcPoint(viewProjTransform, *position); entityPointNdc)
					{
						float distance = glm::distance(glm::vec2(*entityPointNdc), pointNdc);
						if (distance < pickedEntityDistance)
						{
							pickedObject = PickedSceneObject({
								entity,
								*position
								});
							pickedEntityDistance = distance;
						}
					}
				}
			}
		}

		if (!pickedObject)
		{
			glm::dmat4 invViewProjTransform = glm::inverse(viewProjTransform);
			glm::dvec3 origin(invViewProjTransform[3][0], invViewProjTransform[3][1], invViewProjTransform[3][2]);
			pickedObject = pickPointOnPlanet(*world, origin, invViewProjTransform, pointNdc);
		}

		return pickedObject;
	};
}