/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

//! @param invViewProjTransform converts from projection space to world space (inverse of viewProjection matrix)
//! @param pointNdc is a point in the viewport in Normalized Device Coordinates:
//!  * x axis is left to right viewport edge in range [0, 1],
//!  * y axis is top to bottom viewport edge in range [0, 1]
std::optional<skybolt::sim::Vector3> pickPointOnPlanet(const skybolt::sim::World& world, const skybolt::sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc);

using EntitySelectionPredicate = std::function<bool(const skybolt::sim::Entity&)>;
inline bool EntitySelectionPredicateAlways(const skybolt::sim::Entity&) { return true; };

//! @param viewProjTransform converts from world space to projection space
//! @param pointNdc is a point in the viewport in Normalized Device Coordinates:
//!  * x axis is left to right viewport edge in range [0, 1],
//!  * y axis is top to bottom viewport edge in range [0, 1]
using SceneObjectPicker = std::function<std::optional<PickedSceneObject>(const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, float pickRadiusNdc, const EntitySelectionPredicate& predicate)>;

SceneObjectPicker createSceneObjectPicker(const skybolt::sim::World* world);