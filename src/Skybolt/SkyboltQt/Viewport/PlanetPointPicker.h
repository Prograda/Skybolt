/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <functional>
#include <optional>

struct PickedEntity
{
	skybolt::sim::EntityPtr entity; //!< The picked entity
	skybolt::sim::Vector3 position; //!< Position of the picked point
};

//! @param invViewProjTransform converts from projection space to world space (inverse of viewProjection matrix)
//! @param pointNdc is a point in the viewport in Normalized Device Coordinates:
//!  * x axis is left to right viewport edge in range [0, 1],
//!  * y axis is top to bottom viewport edge in range [0, 1]
std::optional<PickedEntity> pickPointOnPlanet(const skybolt::sim::World& world, const skybolt::sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc);

std::optional<skybolt::sim::Vector3> pickPointOnPlanet(const skybolt::sim::Entity& entity, const skybolt::sim::PlanetComponent& planet, const skybolt::sim::Vector3& viewPosition, const skybolt::sim::Vector3& viewDirection);
