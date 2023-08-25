/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/Scenario/ScenarioObjectPredicate.h"
#include "Sprocket/Scenario/ScenarioObjectTypeMap.h"
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <functional>
#include <optional>

struct PickedScenarioObject
{
	ScenarioObjectPtr object; //!< The picked object
	skybolt::sim::Vector3 position; //!< Position of the pick location
};

//! @param viewProjTransform converts from world space to projection space
//! @param pointNdc is a point in the viewport in Normalized Device Coordinates:
//!  * x axis is left to right viewport edge in range [0, 1],
//!  * y axis is top to bottom viewport edge in range [0, 1]
using ScenarioObjectPicker = std::function<std::optional<PickedScenarioObject>(const skybolt::sim::Vector3& viewPosition, const glm::dmat4& viewProjTransform, const glm::vec2& pointNdc, const ScenarioObjectPredicate& predicate)>;

ScenarioObjectPicker createScenarioObjectPicker(const ScenarioObjectTypeMap& entityObjectTypes);