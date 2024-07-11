/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetFeaturesHelpers.h"
#include "PlanetFeaturesSource.h"
#include <SkyboltSim/Spatial/GreatCircle.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace skybolt::math;
using skybolt::sim::LatLon;
using skybolt::sim::LatLonAlt;

namespace skybolt {
namespace mapfeatures {

static double getEndAzimuth(const Road& road, RoadJunction::StartOrEnd startOrEnd)
{
	assert(road.points.size() >= 2);

	if (startOrEnd == RoadJunction::StartOrEnd::End)
	{
		return sim::calcBearing(toLatLon(road.points[road.points.size() - 2]), toLatLon(road.points.back()));
	}
	else // start
	{
		return sim::calcBearing(toLatLon(road.points[1]), toLatLon(road.points.front()));
	}
}

void joinRoadsAtJunction(const RoadJunction& junction)
{
	if (junction.roads.size() == 2)
	{
		// Join two segments directly
		auto& r1 = junction.roads.front();
		auto& r2 = junction.roads.back();

		r1.road->endControlPoints[int(r1.startOrEnd)] = r2.road->points[(r2.startOrEnd == RoadJunction::StartOrEnd::End) ? r2.road->points.size() - 2 : 1];
		r2.road->endControlPoints[int(r2.startOrEnd)] = r1.road->points[(r1.startOrEnd == RoadJunction::StartOrEnd::End) ? r1.road->points.size() - 2 : 1];

		int endLanes = std::min(r1.road->laneCount, r2.road->laneCount);
		r1.road->endLaneCounts[int(r1.startOrEnd)] = endLanes;
		r2.road->endLaneCounts[int(r2.startOrEnd)] = endLanes;
	}
	else if (junction.roads.size() > 2)
	{
		std::vector<int> roadIndices;
		for (size_t i = 0; i < junction.roads.size(); ++i)
		{
			roadIndices.push_back(i);
		}

		// Sort roads from least to most lanes
		std::sort(roadIndices.begin(), roadIndices.end(), [&](int i, int j) {
			return junction.roads[i].road->laneCount < junction.roads[j].road->laneCount;
		});

		const auto& biggestRoad = junction.roads[roadIndices.back()];

		roadIndices.pop_back(); // remove the biggest road because we will leave it as is

		// If the biggest road splits into smaller roads
		if (!roadIndices.empty())
		{
			double biggestRoadAzimuth = getEndAzimuth(*biggestRoad.road, biggestRoad.startOrEnd);
			double perpendicular = biggestRoadAzimuth + math::halfPiD();

			// Find turn angles of the smaller roads
			std::vector<double> turns;
			for (const auto& road : junction.roads)
			{
				double azimuth = getEndAzimuth(*road.road, road.startOrEnd) + math::piD();
				turns.push_back(math::calcSmallestAngleFromTo(biggestRoadAzimuth, azimuth));
			}

			// Ignore roads that join too perpendicularly
			boost::remove_erase_if(roadIndices, [&](int i) {
				return (std::abs(math::halfPiD() - std::abs(turns[i])) < 40 * math::degToRadD());
			});

			// Reorder smaller roads from most to least turn angle
			std::sort(roadIndices.begin(), roadIndices.end(), [&](int i, int j) {
				return std::abs(turns[i]) > std::abs(turns[j]);
			});

			// Displace smaller roads so that all lanes are connected to an unoccupied lane in the larger road
			std::set<int> unoccupiedLanes;
			for (int i = 0; i < biggestRoad.road->laneCount; ++i)
			{
				unoccupiedLanes.insert(i);
			}

			for (int i : roadIndices)
			{
				const auto& road = junction.roads[i];
				float turn = turns[i];

				// Find first lane in biggest road to connect to
				int firstLane;
				if (turn < 0) // left turn
				{
					firstLane = unoccupiedLanes.empty() ? 0 : *unoccupiedLanes.begin();
				}
				else // right turn
				{
					int lastLane = unoccupiedLanes.empty() ? (biggestRoad.road->laneCount - 1) : *(--unoccupiedLanes.end());
					firstLane = lastLane - road.road->laneCount + 1;
				}

				// Ensure road is not offset beyond bounds of biggest road
				int maxFirstLane = biggestRoad.road->laneCount - road.road->laneCount;
				firstLane = std::clamp(firstLane, 0, maxFirstLane);

				// Apply offset
				double offset = (double(firstLane) + double(road.road->laneCount)  * 0.5) / biggestRoad.road->laneCount;
				offset = (offset - 0.5) * biggestRoad.road->width;

				sim::LatLonAlt& point = road.road->points[(road.startOrEnd == RoadJunction::StartOrEnd::End) ? road.road->points.size() - 1 : 0];
				sim::LatLon point2d = sim::moveDistanceAndBearing(toLatLon(point), offset, perpendicular);
				point.lat = point2d.lat;
				point.lon = point2d.lon;

				// Add a new end vertex past the current end of the small road to ensure the road joins with correct alignment to the biggest road.
				// Without doing this there can be a small gap due to change slight in road angle at the junction.
				{
					double offset = road.road->width * 0.5;
					if (std::abs(turn) < math::halfPiD()) // if road merges in opposite direction to the biggest road, flip offset
					{
						offset = -offset;
					}

					point2d = sim::moveDistanceAndBearing(point2d, offset, biggestRoadAzimuth);
					sim::LatLonAlt newPoint = point;
					newPoint.lat = point2d.lat;
					newPoint.lon = point2d.lon;

					if (road.startOrEnd == RoadJunction::StartOrEnd::End)
					{
						road.road->points.push_back(newPoint);
					}
					else
					{
						road.road->points.insert(road.road->points.begin(), newPoint);
					}
				}

				// Mark lanes as occupied
				for (int j = firstLane; j < firstLane + road.road->laneCount; ++j)
				{
					unoccupiedLanes.erase(j);
				}
			}
		}
	}
}

} // namespace mapfeatures
} // namespace skybolt