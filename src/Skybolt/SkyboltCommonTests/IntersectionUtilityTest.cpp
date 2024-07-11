/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#define CATCH_CONFIG_MAIN
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <catch2/catch.hpp>

using namespace skybolt;

constexpr float epsilon = 1e-8f;

TEST_CASE("Intersect grid along X axis")
{
	Grid grid;
	grid.origin = glm::vec2(1.0f, 0.f);
	grid.cellSize = glm::vec2(2.0f, 2.0f);
	grid.countX = 10;
	grid.countY = 15;

	glm::vec2 origin(1.2f, 2.5f);
	glm::vec2 direction(1.0f, 0.0f);
	float length = 6.0f;

	std::vector<glm::ivec2> intersectedCells;

	intersectRayGrid(grid, origin, direction, length, intersectedCells);

	REQUIRE(intersectedCells.size() == 4);
	CHECK(intersectedCells[0] == glm::ivec2(0, 1));
	CHECK(intersectedCells[1] == glm::ivec2(1, 1));
	CHECK(intersectedCells[2] == glm::ivec2(2, 1));
	CHECK(intersectedCells[3] == glm::ivec2(3, 1));
}

TEST_CASE("Intersect grid along Y axis")
{
	Grid grid;
	grid.origin = glm::vec2(1.0f, 0.f);
	grid.cellSize = glm::vec2(2.0f, 2.0f);
	grid.countX = 10;
	grid.countY = 15;

	glm::vec2 origin(5.2f, 2.5f);
	glm::vec2 direction(0.0f, 1.0f);
	float length = 6.0f;
	
	std::vector<glm::ivec2> intersectedCells;

	intersectRayGrid(grid, origin, direction, length, intersectedCells);

	REQUIRE(intersectedCells.size() == 4);
	CHECK(intersectedCells[0] == glm::ivec2(2, 1));
	CHECK(intersectedCells[1] == glm::ivec2(2, 2));
	CHECK(intersectedCells[2] == glm::ivec2(2, 3));
	CHECK(intersectedCells[3] == glm::ivec2(2, 4));
}

TEST_CASE("Intersect grid diagonally")
{
	Grid grid;
	grid.origin = glm::vec2(0.f, 0.f);
	grid.cellSize = glm::vec2(1.0f, 1.0f);
	grid.countX = 10;
	grid.countY = 15;

	glm::vec2 origin(5.5f, 4.1f);
	glm::vec2 direction = glm::normalize(glm::vec2(-1.0f, -1.0f));
	float length = 1.0f;

	std::vector<glm::ivec2> intersectedCells;

	intersectRayGrid(grid, origin, direction, length, intersectedCells);

	REQUIRE(intersectedCells.size() == 3);
	CHECK(intersectedCells[0] == glm::ivec2(5, 4));
	CHECK(intersectedCells[1] == glm::ivec2(5, 3));
	CHECK(intersectedCells[2] == glm::ivec2(4, 3));
}

TEST_CASE("Intersect ray vs sphere")
{
	// Test hit
	glm::vec3 r0(4,2,3);
	glm::vec3 rd(-1,0,0);
	glm::vec3 s0(1,2,3);
	float sr = 2;
	auto result = intersectRaySphere(r0, rd, s0, sr);
	REQUIRE(result);
	CHECK(result->first == Approx(1.0).margin(epsilon));
	CHECK(result->second == Approx(5.0).margin(epsilon));

	// Test miss
	result = intersectRaySphere(r0, glm::vec3(0,1,0), s0, sr);
	CHECK(!result);
}

TEST_CASE("Nearest distance on line to point")
{
	// Point near middle of line
	float r = nearestNormalizedDistanceOnLineToPoint(glm::vec3(1,0,0), glm::vec3(3,0,0), glm::vec3(2,1,0));
	CHECK(r == 0.5f);

	// Point beyond start of line
	r = nearestNormalizedDistanceOnLineToPoint(glm::vec3(1,0,0), glm::vec3(3,0,0), glm::vec3(0,1,0));
	CHECK(r == 0.f);

	// Point beyond end of line
	r = nearestNormalizedDistanceOnLineToPoint(glm::vec3(1,0,0), glm::vec3(3,0,0), glm::vec3(4,1,0));
	CHECK(r == 1.f);
}

TEST_CASE("Nearest distances on rays")
{
	// Test rays with same origin intersect at origin
	// Test perpendicular rays that intersect
	auto r = nearestDistancesOnRays(glm::vec3(1,2,3), glm::vec3(0,1,0), glm::vec3(1,2,3), glm::vec3(1,0,0));
	REQUIRE(r.has_value());
	CHECK(*r == std::make_pair(0.f, 0.f));

	// Test perpendicular rays that intersect
	r = nearestDistancesOnRays(glm::vec3(2,0,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(1,0,0));
	REQUIRE(r.has_value());
	CHECK(*r == std::make_pair(1.f, 2.f));

	// Test perpendicular rays that miss
	r = nearestDistancesOnRays(glm::vec3(2,0,0), glm::vec3(0,1,0), glm::vec3(0,1,3), glm::vec3(1,0,0));
	REQUIRE(r.has_value());
	CHECK(*r == std::make_pair(1.f, 2.f));

	// Test parallel rays have no intersection
	r = nearestDistancesOnRays(glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(1,0,0));
	REQUIRE(!r.has_value());

	// Test anti-parallel rays have no intersection
	r = nearestDistancesOnRays(glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(-1,0,0));
	REQUIRE(!r.has_value());
}
