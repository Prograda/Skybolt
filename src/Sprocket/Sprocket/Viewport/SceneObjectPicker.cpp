#include "SceneObjectPicker.h"
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
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