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

sim::Vector3 screenToWorldDirection(const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	glm::vec4 point(pointNdc.x * 2.0 - 1.0, pointNdc.y * 2.0 - 1.0, 0.99, 1);
	point = invViewProjTransform * point;
	point.x /= point.w;
	point.y /= point.w;
	point.z /= point.w;
	sim::Vector3 pointVec3 = point;
	return glm::normalize(sim::Vector3(point) - origin);
}

std::optional<sim::Vector3> pickPointOnPlanet(const skybolt::sim::World& world, const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	sim::Vector3 dir = screenToWorldDirection(origin, invViewProjTransform, pointNdc);
	if (sim::Entity* entity = sim::findNearestEntityWithComponent<sim::PlanetComponent>(world.getEntities(), origin); entity)
	{
		if (auto position = getPosition(*entity); position)
		{
			auto component = entity->getFirstComponentRequired<sim::PlanetComponent>();
			if (auto r = intersectRaySphere(origin, dir, *position, component->radius); r)
			{
				return origin + glm::inverse(*getOrientation(*entity)) * (dir * double(r->first));
			}
		}
	}
	return std::nullopt;
}

SceneObjectPicker createSceneObjectPicker(const sim::World* world)
{
	return [world](const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, float pickRadiusNdc) -> std::optional<PickedSceneObject> {

		std::optional<PickedSceneObject> pickedObject;
		float pickedEntityDistance = pickRadiusNdc;

		for (const auto& entity : world->getEntities())
		{
			if (auto position = getPosition(*entity); position)
			{
				glm::dvec4 entityPoint = viewProjTransform * glm::dvec4(*position, 1.0);
				if (entityPoint.z > 0)
				{
					glm::vec2 entityPointNdc(entityPoint.x / entityPoint.w, -entityPoint.y / entityPoint.w);
					entityPointNdc = entityPointNdc * 0.5f + glm::vec2(0.5f);
					float distance = glm::distance(entityPointNdc, pointNdc);
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

		return pickedObject;
	};
}