/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/SimMath.h>
#include <optional>

//! @returns transform that converts from world space to view projection space
glm::dmat4 makeViewProjTransform(const skybolt::sim::Vector3& origin, const skybolt::sim::Quaternion& orientation, const skybolt::sim::CameraState& camera, double aspectRatio);

//! NDC coordinates are represented in double precision to ensure sufficient accuracy for z coordinate
std::optional<skybolt::sim::Vector3> worldToScreenNdcPoint(const glm::dmat4& viewProjTransform, const skybolt::sim::Vector3& pointWorldSpace);
skybolt::sim::Vector3 screenToWorldPoint(const glm::dmat4& invViewProjTransform, const skybolt::sim::Vector3& pointNdc);
skybolt::sim::Vector3 screenToWorldDirection(const skybolt::sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc);
