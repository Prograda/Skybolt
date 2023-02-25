/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "IntersectionUtility.h"

namespace skybolt
{

using namespace math;

bool intersectRayAabb(const glm::vec3 &origin, const glm::vec3 &direction, const Box3 &box, float &hitDistance)
{
	// do collision detection using Slabs method
	hitDistance = negInfinity();
	float tFar = posInfinity();

	const glm::vec3 &minBound = box.minimum;
	const glm::vec3 &maxBound = box.maximum;

	for (int i=0; i<3; i++)
	{
		if (direction[i] == 0.0f) // if ray is parallel
		{
			if (origin[i] > minBound[i] && origin[i] < maxBound[i]) // if origin is between planes
				continue;
			else
				return false;
		}

		float t1 = (minBound[i] - origin[i]) / direction[i];
		float t2 = (maxBound[i] - origin[i]) / direction[i];

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}

		if (t1 > hitDistance)
			hitDistance = t1;

		if (t2 < tFar)
			tFar = t2;

		if (hitDistance > tFar) // box missed
			return false;

		if (tFar < 0.0f) // box behind ray
			return false;
	}

	return true;
}

bool intersectRayAabb(const glm::vec2 &origin, const glm::vec2 &direction, const Box2 &box, float &hitDistance)
{
	// do collision detection using Slabs method
	hitDistance = negInfinity();
	float tFar = posInfinity();

	const glm::vec2 &minBound = box.minimum;
	const glm::vec2 &maxBound = box.maximum;

	for (int i = 0; i<2; i++)
	{
		if (direction[i] == 0.0f) // if ray is parallel
		{
			if (origin[i] > minBound[i] && origin[i] < maxBound[i]) // if origin is between planes
				continue;
			else
				return false;
		}

		float t1 = (minBound[i] - origin[i]) / direction[i];
		float t2 = (maxBound[i] - origin[i]) / direction[i];

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}

		if (t1 > hitDistance)
			hitDistance = t1;

		if (t2 < tFar)
			tFar = t2;

		if (hitDistance > tFar) // box missed
			return false;

		if (tFar < 0.0f) // box behind ray
			return false;
	}

	return true;
}

void intersectRayGrid(const Grid& grid, const glm::vec2& origin, const glm::vec2& direction, float length, std::vector<glm::ivec2>& intersectedCells)
{
	// Early out with ray vs AABB
	float t;
	Box2 box(glm::vec2(0.0f), grid.cellSize * glm::vec2(grid.countX, grid.countY));
	if (!intersectRayAabb(origin, direction, box, t))
		return;

	glm::vec2 pos = origin - grid.origin;

	// Make the place where ray intersects terrain AABB the new start position
	if (t > 0.0)
	{
		pos += direction * t;
		length -= t;
	}

	glm::vec2 rcpCellSize = 1.0f / grid.cellSize;

	float xF = pos.x * rcpCellSize.x;
	float yF = pos.y * rcpCellSize.y;

	int x = clamp((int)xF, 0, grid.countX - 1);
	int y = clamp((int)yF, 0, grid.countY - 1);

	float interpFactorX = xF - x;
	float interpFactorY = yF - y;

	int stepX = (int)glm::sign(direction.x);
	int stepY = (int)glm::sign(direction.y);

	float tDeltaX = (direction.x != 0.0f) ? 1.f / glm::abs(direction.x) : 1e20f;
	float tDeltaY = (direction.y != 0.0f) ? 1.f / glm::abs(direction.y) : 1e20f;

	float maxX, maxY;
	int boundMinX, boundMinY, boundMaxX, boundMaxY;
	
	if (direction.x > 0)
	{
		maxX = 1.0f - interpFactorX;
		boundMinX = 0;
		boundMaxX = std::min(x + (int)glm::ceil(direction.x * length * rcpCellSize.x), grid.countX-1);
	}
	else
	{
		maxX = interpFactorX;
		boundMinX = std::max(x + (int)glm::floor(direction.x * length * rcpCellSize.x), 0);
		boundMaxX = grid.countX;
	}

	if (direction.y > 0)
	{
		maxY = 1.0f - interpFactorY;
		boundMinY = 0;
		boundMaxY = std::min(y + (int)glm::ceil(direction.y * length * rcpCellSize.y), grid.countY-1);
	}
	else
	{
		maxY = interpFactorY;
		boundMinY = std::max(y + (int)glm::floor(direction.y * length * rcpCellSize.y), 0);
		boundMaxY = grid.countY;
	}

	float tMaxX = maxX * tDeltaX;
	float tMaxY = maxY * tDeltaY;

	while (1)
	{
		//if ray has gone outside of bounds, abort
		if (x < boundMinX || x > boundMaxX || y < boundMinY || y > boundMaxY)
			return;

		intersectedCells.push_back(glm::ivec2(x, y));

		//move further along ray and get next cell
		if (tMaxX < tMaxY)
		{
			tMaxX += tDeltaX;
			x += stepX;
		}
		else
		{
			tMaxY += tDeltaY;
			y += stepY;
		}
	}
}

std::optional<std::pair<float, float>> intersectRaySphere(const glm::vec3& r0, const glm::vec3& rd, const glm::vec3& s0, float sr)
{
    glm::vec3 s0_r0 = r0 - s0;
    float b = 2.0f * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);

	float det = b*b - 4.0f*c;
    if (det < 0.0)
	{
		return std::nullopt;
    }
	float sqrtDet = sqrt(det);
	return std::make_pair((-b - sqrtDet) * 0.5f, (-b + sqrtDet) * 0.5f);
}

std::optional<std::pair<float, float>> intersectRaySegmentSphere(const glm::vec3& r0, const glm::vec3& rd, float rl, const glm::vec3& s0, float sr)
{
	auto result = intersectRaySphere(r0, rd, s0, sr);
	if (result && result->first >= 0 && result->first < rl)
	{
		return result;
	}
	return std::nullopt;
}

} // namespace skybolt
