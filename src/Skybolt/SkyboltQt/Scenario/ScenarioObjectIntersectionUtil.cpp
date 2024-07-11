/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioObjectIntersectionUtil.h"
#include "SkyboltQt/Viewport/ScreenTransformUtil.h"

#include <SkyboltCommon/Math/IntersectionUtility.h>

using namespace skybolt;

std::optional<sim::Vector3> intersectRayWithIcon(const sim::Vector3& rayOrigin, const sim::Vector3& rayDirection, const glm::dmat4& viewProjTransform, const sim::Vector3& objectPosition)
{
	sim::Vector3 tangent, binormal;
	sim::getOrthonormalBasis(rayDirection, tangent, binormal);

	auto entityPositionNdc0 = worldToScreenNdcPoint(viewProjTransform, objectPosition);
	auto entityPositionNdc1 = worldToScreenNdcPoint(viewProjTransform, objectPosition + tangent);
	if (entityPositionNdc0 && entityPositionNdc1)
	{
		double ndcSizePerWorldUnit = glm::distance(*entityPositionNdc0, *entityPositionNdc1);

		static constexpr double ndcEntityRadius = 0.03;
		double worldRadius = ndcEntityRadius / ndcSizePerWorldUnit;

		if (auto r = intersectRaySphere(rayOrigin, rayDirection, objectPosition, worldRadius); r)
		{
			return rayOrigin + rayDirection * double(r->first);
		}
	}
	return std::nullopt;
}