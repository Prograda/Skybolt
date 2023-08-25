/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SimMath.h>

#include <optional>

//! Intersects a ray aginst a an icon at a world position.
//! The icon has a fixed size in screen space.
std::optional<skybolt::sim::Vector3> intersectRayWithIcon(const skybolt::sim::Vector3& rayOrigin, const skybolt::sim::Vector3& rayDirection, const glm::dmat4& viewProjTransform, const skybolt::sim::Vector3& objectPosition);