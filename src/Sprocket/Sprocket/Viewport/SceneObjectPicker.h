#pragma once

#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <functional>
#include <optional>

struct PickedSceneObject
{
	skybolt::sim::EntityPtr entity; //!< The picked entity
	skybolt::sim::Vector3 position; //!< Position of the pick location
};

//! @returns transform that converts from world space to view projection space
glm::dmat4 makeViewProjTransform(const skybolt::sim::Vector3& origin, const skybolt::sim::Quaternion& orientation, const skybolt::sim::CameraState& camera, double aspectRatio);

skybolt::sim::Vector3 screenToWorldDirection(const skybolt::sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc);

std::optional<skybolt::sim::Vector3> pickPointOnPlanet(const skybolt::sim::World& world, const skybolt::sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc);

//! @param viewProjTransform converts from world space to view projection space
//! @param pointNdc is a point in the viewport in Normalized Device Coordinates:
//!  - x axis is left to right viewport edge in range [0, 1],
//!  - y axis is top to bottom viewport edge in range [0, 1]
using SceneObjectPicker = std::function<std::optional<PickedSceneObject>(const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, float pickRadiusNdc)>;

SceneObjectPicker createSceneObjectPicker(const skybolt::sim::World* world);