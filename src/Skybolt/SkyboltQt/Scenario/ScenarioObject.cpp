/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObject.h"
#include "ScenarioObjectIntersectionUtil.h"

std::optional<skybolt::sim::Vector3> ScenarioObject::intersectRay(const skybolt::sim::Vector3& origin, const skybolt::sim::Vector3& dir, const glm::dmat4& viewProjTransform) const
{
	if (auto position = getWorldPosition(); position)
	{
		return intersectRayWithIcon(origin, dir, viewProjTransform, *position);
	}
	return std::nullopt;
}
