/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScreenTransformUtil.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Components/CameraComponent.h>

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

std::optional<sim::Vector3> worldToScreenNdcPoint(const glm::dmat4& viewProjTransform, const skybolt::sim::Vector3& pointWorldSpace)
{
	glm::dvec4 entityPoint = viewProjTransform * glm::dvec4(pointWorldSpace, 1.0);
	if (entityPoint.z > 0)
	{
		sim::Vector3 entityPointNdc(entityPoint.x / entityPoint.w, -entityPoint.y / entityPoint.w, entityPoint.z / entityPoint.w); // Note: y axis is flipped
		return entityPointNdc * 0.5 + sim::Vector3(0.5);
	}
	return std::nullopt;
}

skybolt::sim::Vector3 screenToWorldPoint(const glm::dmat4& invViewProjTransform, const sim::Vector3& pointNdc)
{
	glm::dvec4 point(pointNdc.x * 2.0 - 1.0, pointNdc.y * -2.0 + 1.0, pointNdc.z * 2.0 - 1.0, 1); // Note: y axis is flipped
	point = invViewProjTransform * point;
	point.x /= point.w;
	point.y /= point.w;
	point.z /= point.w;
	return point;
}

sim::Vector3 screenToWorldDirection(const sim::Vector3& origin, const glm::dmat4& invViewProjTransform, const glm::vec2& pointNdc)
{
	sim::Vector3 point = screenToWorldPoint(invViewProjTransform, sim::Vector3(pointNdc.x, pointNdc.y, 0.99));
	return glm::normalize(point - origin);
}
