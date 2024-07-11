/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>

#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesHelpers.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

using namespace skybolt;
using namespace skybolt::mapfeatures;
using namespace skybolt::sim;

TEST_CASE("Join two roads at junction")
{
	auto roadA = std::make_shared<Road>();
	roadA->laneCount = 2;
	roadA->points = { LatLonAlt(0, 0, 0), LatLonAlt(0.01, 0, 0)};

	auto roadB = std::make_shared<Road>();
	roadB->laneCount = 2;
	roadB->points = {LatLonAlt(0, 0.01, 0), LatLonAlt(0, 0, 0)};

	RoadJunction junction;
	junction.roads = {{ roadA, RoadJunction::StartOrEnd::Start }, { roadB, RoadJunction::StartOrEnd::End }};

	joinRoadsAtJunction(junction);

	CHECK(roadA->endControlPoints[0] == roadB->points[0]);
	CHECK(roadA->endLaneCounts[0] == roadB->laneCount);
	CHECK(roadA->endLaneCounts[1] == Road::noJunction);

	CHECK(roadB->endControlPoints[1] == roadA->points[1]);
	CHECK(roadB->endLaneCounts[0] == Road::noJunction);
	CHECK(roadB->endLaneCounts[1] == roadA->laneCount);
}

using RoadPtr = std::shared_ptr<Road>;

constexpr float laneWidth = 10;

static RoadPtr createLeftRoadWithLanes(int laneCount)
{
	auto road = std::make_shared<Road>();
	road->points = {LatLonAlt(0, 0, 0), LatLonAlt(-0.01, 0.01, 0)};
	road->laneCount = laneCount;
	road->width = laneWidth * laneCount;
	return road;
}

static RoadPtr createRightRoadWithLanes(int laneCount)
{
	auto road = std::make_shared<Road>();
	road->points = {LatLonAlt(0, 0, 0), LatLonAlt(-0.01, -0.01, 0)};
	road->laneCount = laneCount;
	road->width = laneWidth * laneCount;
	return road;
}

static void joinRoads(const std::vector<RoadJunction::Item>& roads)
{
	RoadJunction junction;
	junction.roads = roads;
	joinRoadsAtJunction(junction);
}

TEST_CASE("Join two small roads to a big road")
{
	auto bigRoad = std::make_shared<Road>();
	bigRoad->laneCount = 5;
	bigRoad->width = bigRoad->laneCount * laneWidth;
	bigRoad->points = {LatLonAlt(0, 0, 0), LatLonAlt(0.01, 0, 0)};

	SECTION("Small roads fit side-by-side without overlapping")
	{
		auto smallRoadLeft = createLeftRoadWithLanes(3);
		auto smallRoadRight = createRightRoadWithLanes(2);

		joinRoads({
			{ bigRoad, RoadJunction::StartOrEnd::Start },
			{ smallRoadLeft, RoadJunction::StartOrEnd::Start },
			{ smallRoadRight, RoadJunction::StartOrEnd::Start }
		});

		// Big road is unchanged
		CHECK(bigRoad->points[0] == LatLonAlt(0, 0, 0));

		// Left small road moved to left lanes
		CHECK(smallRoadLeft->points[1].lon > 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadLeft->points[1])) == Approx(laneWidth * 1).epsilon(1e-8));

		// Right small road moved to right lanes
		CHECK(smallRoadRight->points[1].lon < 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadRight->points[1])) == Approx(laneWidth * 1.5).epsilon(1e-8));

		// Small roads had second vertex added to ensure road aligns with big road
		CHECK(smallRoadLeft->points.size() == 3);
		CHECK(smallRoadLeft->points[0].lon == Approx(smallRoadLeft->points[1].lon).epsilon(1e-8));
		CHECK(smallRoadLeft->points[0].lat > smallRoadLeft->points[1].lat);

		CHECK(smallRoadRight->points.size() == 3);
		CHECK(smallRoadRight->points[0].lon == Approx(smallRoadRight->points[1].lon).epsilon(1e-8));
		CHECK(smallRoadRight->points[0].lat > smallRoadRight->points[1].lat);
	}

	SECTION("Two small roads overlap")
	{
		auto smallRoadLeft = createLeftRoadWithLanes(3);
		auto smallRoadRight = createRightRoadWithLanes(4);

		joinRoads({
			{ bigRoad, RoadJunction::StartOrEnd::Start },
			{ smallRoadLeft, RoadJunction::StartOrEnd::Start },
			{ smallRoadRight, RoadJunction::StartOrEnd::Start }
		});

		// Left small road moved to left lanes
		CHECK(smallRoadLeft->points[0].lon > 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadLeft->points[1])) == Approx(laneWidth * 1).epsilon(1e-8));

		// Right small road moved to right lanes
		CHECK(smallRoadRight->points[0].lon < 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadRight->points[1])) == Approx(laneWidth * 0.5).epsilon(1e-8));
	}

	SECTION("Two small roads overlap and turn in same direction")
	{
		auto smallRoadLeft = createLeftRoadWithLanes(3);
		auto smallRoadRight = createRightRoadWithLanes(4);

		smallRoadLeft->points = {LatLonAlt(0, 0, 0), LatLonAlt(-0.01, 0.011, 0)};
		smallRoadRight->points = {LatLonAlt(0, 0, 0), LatLonAlt(-0.01, 0.01, 0)};

		joinRoads({
			{ bigRoad, RoadJunction::StartOrEnd::Start },
			{ smallRoadLeft, RoadJunction::StartOrEnd::Start },
			{ smallRoadRight, RoadJunction::StartOrEnd::Start }
		});

		// Left small road moved to left lanes
		CHECK(smallRoadLeft->points[0].lon > 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadLeft->points[1])) == Approx(laneWidth * 1).epsilon(1e-8));

		// Right small road moved to right lanes
		CHECK(smallRoadRight->points[0].lon < 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadRight->points[1])) == Approx(laneWidth * 0.5).epsilon(1e-8));
	}

	SECTION("Small road ignored if it turns too perpendicularly")
	{
		auto smallRoadLeft = createLeftRoadWithLanes(3);
		auto smallRoadRight = createRightRoadWithLanes(4);

		smallRoadLeft->points = {LatLonAlt(0, 0, 0), LatLonAlt(0, 0.01, 0)};
		smallRoadRight->points = {LatLonAlt(0, 0, 0), LatLonAlt(-0.01, -0.01, 0)};

		joinRoads({
			{ bigRoad, RoadJunction::StartOrEnd::Start },
			{ smallRoadLeft, RoadJunction::StartOrEnd::Start },
			{ smallRoadRight, RoadJunction::StartOrEnd::Start }
		});

		// Left small road moved to left lanes
		CHECK(smallRoadLeft->points[0].lon == 0);

		// Right small road moved to right lanes
		CHECK(smallRoadRight->points[0].lon < 0);
		CHECK(calcDistance(toLatLon(bigRoad->points[0]), toLatLon(smallRoadRight->points[1])) == Approx(laneWidth * 0.5).epsilon(1e-8));
	}
}
