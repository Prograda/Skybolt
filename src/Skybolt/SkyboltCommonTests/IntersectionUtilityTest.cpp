/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#define CATCH_CONFIG_MAIN
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <catch2/catch.hpp>

using namespace skybolt;

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
	CHECK(result->first == Approx(1.0).margin(1e-8f));
	CHECK(result->second == Approx(5.0).margin(1e-8f));

	// Test miss
	result = intersectRaySphere(r0, glm::vec3(0,1,0), s0, sr);
	CHECK(!result);
}