/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FeaturesConverter.h"
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesHelpers.h>
#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <readosm.h>
#include <sstream>
#include <map>
#include <set>
#include <iostream>
#include <boost/algorithm/string.hpp>  
#include <boost/lexical_cast.hpp>

using namespace skybolt::math;
using skybolt::sim::LatLon;

namespace skybolt {
namespace mapfeatures {

const float buildingLevelHeight = 3.9f;
const float buildingGroundLevelHeight = 2.0f * buildingLevelHeight;
const double earthRadius = 6378100.0;

static float calcBuildingHeightFromLevelCount(int levelCount)
{
	assert(levelCount >= 1);
	return buildingGroundLevelHeight * (levelCount - 1) + buildingLevelHeight;
}

static LatLonAltPoints toLatLonAlt(const LatLonPoints& points, const sim::PlanetAltitudeProvider& provider)
{
	LatLonAltPoints result(points.size());
	int i = 0;
	for (const LatLon& point : points)
	{
		result[i] = toLatLonAlt(point, provider.getAltitude(point).altitude);
		++i;
	}
	return result;
}

static LatLonAltPoints toLatLonWithMinAlt(const LatLonPoints& points, const sim::PlanetAltitudeProvider& provider)
{
	LatLonAltPoints result(points.size());
	double alt = math::posInfinity();
	for (const LatLon& point : points)
	{
		alt = std::min(alt, provider.getAltitude(point).altitude);
	}

	int i = 0;
	for (const LatLon& point : points)
	{
		result[i] = toLatLonAlt(point, alt);
		++i;
	}
	return result;
}

struct ParserData
{
	struct Way
	{
		std::vector<long long> nodes;
	};

	std::map<long long, LatLon> nodes;
	std::map<long long, Way> ways;

	struct ParserAirport
	{
		std::string name;
		LatLonBounds bounds;
		std::vector<LatLonPoints> areaPolygons;
	};
	std::vector<ParserAirport> airports;
	std::vector<Airport::Runway> runways;

	std::vector<FeaturePtr> features;
	std::map<long long, RoadJunction> nodeRoadJunctions;

	const sim::PlanetAltitudeProvider* altitudeProvider;
};

float getHighwayRoadWidth(int laneCount) {return 3.7f * laneCount;}
float getResidentialRoadWidth(int laneCount) {return 3.5f * laneCount;}

static int parseNode(const void* user_data, const readosm_node* node)
{
	if (node->latitude == READOSM_UNDEFINED)
		throw skybolt::Exception("Undefined latitude");
	if (node->longitude == READOSM_UNDEFINED)
		throw skybolt::Exception("Undefined longitude");

	ParserData& data = *(ParserData*)user_data;
	data.nodes[node->id] = LatLon(node->latitude * degToRadD(), node->longitude * degToRadD());

	if (data.nodes.size() % 1000000 == 0)
		printf("Loaded %zu nodes\n", data.nodes.size());

	return READOSM_OK;
}

template <class T>
const readosm_tag* getTag(const T& object, const char* key)
{
	for (int i = 0; i < object.tag_count; i++)
	{
		const readosm_tag* tag = object.tags + i;
		if (strcmp(tag->key, key) == 0)
		{
			return tag;
		}
	}
	return nullptr;
}

template <class T>
const char* getTagValue(const T& object, const char* key)
{
	const readosm_tag* tag = getTag(object, key);
	if (tag)
	{
		return tag->value;
	}
	return nullptr;
}

template <class T>
std::string getTagValueString(const T& object, const char* key)
{
	const char* value = getTagValue(object, key);
	return value ? std::string(value) : "";
}

enum class Units
{
	Meters,
	Feet
};

std::map<Units, double> conversionsToMeters = { {Units::Meters, 1.0}, {Units::Feet, 0.3048} };

Units toUnits(const std::string& str)
{
	if (str == "ft")
	{
		return Units::Feet;
	}
	else if (str == "m")
	{
		return Units::Meters;
	}
	else
	{
		throw skybolt::Exception("Unknown units '" + std::string(str) + "'");
	}
}

static double convert(double value, Units from, Units to)
{
	return value * conversionsToMeters[from] / conversionsToMeters[to];
}

static double convertToUnits(const char* valueWithUnits, Units resultUnits)
{
	std::vector<std::string> vec;
	boost::split(vec, valueWithUnits, boost::is_any_of(" "), boost::token_compress_on);
	if (vec.size() != 1 && vec.size() != 2)
	{
		throw skybolt::Exception("Could parse value '" + std::string(valueWithUnits) + "' because it is formatted incorrectly");
	}

	double value = std::stod(vec.front());
	if (vec.size() == 1)
	{
		return value;
	}
	else
	{
		return convert(value, toUnits(vec.back()), resultUnits);
	}
}

template <class T, typename V>
V getTagValueOrDefault(const T& object, const char* key, Units units, const V& defaultValue)
{
	const char* value = getTagValue(object, key);
	if (value)
	{
		return convertToUnits(value, units);
	}
	return defaultValue;
}

static void printTags(const readosm_way& way)
{
	for (int i = 0; i < way.tag_count; i++)
	{
		const readosm_tag* tag = way.tags + i;
		std::cout << tag->key << ": " << tag->value << std::endl;
	}
}

static void readPoints(const readosm_way& way, const ParserData& data, std::vector<LatLon>& points)
{
	for (int i = 0; i < way.node_ref_count; ++i)
	{
		long long nodeId = way.node_refs[i];

		std::map<long long, LatLon>::const_iterator it = data.nodes.find(nodeId);
		if (it == data.nodes.end())
		{
			std::stringstream ss;
			ss << nodeId;
			throw skybolt::Exception("Invalid node ID " + ss.str());
		}
		points.push_back(it->second);
	}
}

static void readPoints(const ParserData::Way& way, const ParserData& data, std::vector<LatLon>& points)
{
	for (size_t i = 0; i < way.nodes.size(); ++i)
	{
		long long nodeId = way.nodes[i];

		std::map<long long, LatLon>::const_iterator it = data.nodes.find(nodeId);
		if (it == data.nodes.end())
		{
			std::stringstream ss;
			ss << nodeId;
			throw skybolt::Exception("Invalid node ID " + ss.str());
		}
		points.push_back(it->second);
	}
}

static double longitudeDifference(double a, double b)
{
	if (abs(a - b) > piD())
	{
		if (a > b)
		{
			return a - (b + piD());
		}
		else
		{
			return a - (b - piD());
		}
	}
	return a - b;
}

bool isClockwise(const std::vector<LatLon>& points)
{
	// From https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order

	float d = 0.0f;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const LatLon& v0 = points[i];
		const LatLon& v1 = points[(i + 1) % points.size()];
		d += longitudeDifference(v1.lon, v0.lon) * (v1.lat + v0.lat);
	}
	return d > 0.0f;
}

void preparePoly(std::vector<LatLon>& points)
{
	// Remove last point if it's a duplicate of first point
	if (points.back() == points.front())
	{
		points.pop_back();
	}

	if (!isClockwise(points))
	{
		std::reverse(points.begin(), points.end());
	}
}

bool isClosed(const std::vector<LatLon>& points)
{
	return (points.size() >= 2 && points.back() == points.front());
}

static int parseWay(const void* user_data, const readosm_way* way)
{
	ParserData& data = *(ParserData*)user_data;
	std::vector<FeaturePtr>& features = data.features;

	ParserData::Way parsedWay;
	for (int i = 0; i < way->node_ref_count; ++i)
	{
		parsedWay.nodes.push_back(way->node_refs[i]);
	}
	data.ways[way->id] = parsedWay;

	const readosm_tag* tag = getTag(*way, "highway");
	if (tag)
	{
		if (strcmp(tag->value, "motorway") == 0
			|| strcmp(tag->value, "motorway_link") == 0
			|| strcmp(tag->value, "trunk") == 0
			|| strcmp(tag->value, "primary") == 0
			|| strcmp(tag->value, "secondary") == 0
			|| strcmp(tag->value, "tertiary") == 0
			|| strcmp(tag->value, "residential") == 0
			)
		{
			if (getTag(*way, "tunnel")) // ignore tunnels
			{
				return READOSM_OK;
			}

			std::shared_ptr<Road> roadPtr = std::make_shared<Road>();
			Road& road = *roadPtr;
			road.laneCount = 2; // by default, assume road has one lane going one way, and another going the opposite way, e.g typical residential street.

			const readosm_tag* lanesTag = getTag(*way, "lanes");
			if (lanesTag)
			{
				road.laneCount = atoi(lanesTag->value);
			}

			bool isResidential = (strcmp(tag->value, "residential") == 0);
			road.width = isResidential ? getResidentialRoadWidth(road.laneCount) : getHighwayRoadWidth(road.laneCount);

			if (road.width > 0.0f)
			{
				LatLonPoints latLonPoints;
				readPoints(*way, data, latLonPoints);

				if (latLonPoints.size() >= 2)
				{
					road.points = toLatLonAlt(latLonPoints, *data.altitudeProvider);
					features.push_back(roadPtr);

					long long startNode = way->node_refs[0];
					long long endNode = way->node_refs[way->node_ref_count - 1];
					data.nodeRoadJunctions[startNode].roads.push_back({roadPtr, RoadJunction::StartOrEnd::Start});
					data.nodeRoadJunctions[endNode].roads.push_back({roadPtr, RoadJunction::StartOrEnd::End});
				}
			}
		}
	}

	tag = getTag(*way, "building");
	if (!tag)
	{
		tag = getTag(*way, "building:part");
	}
	if (tag)
	{
		float height = 0.0f;
		const readosm_tag* heightTag = getTag(*way, "height");
		if (heightTag)
		{
			try
			{
				height = std::stod(heightTag->value);
			}
			catch (const std::invalid_argument&)
			{
#ifdef FEATURES_CONVERTER_STRICT_ERROR_CHECKING
				throw std::runtime_error("Invalid value for building height: '" + std::string(heightTag->value) + "'");
#endif
			}
		}
		else
		{
			const readosm_tag* levelsTag = getTag(*way, "building:levels");
			if (levelsTag)
			{
				int levels = 1;
				try
				{
					levels = std::stoi(levelsTag->value);
				}
				catch (const std::invalid_argument&)
				{
#ifdef FEATURES_CONVERTER_STRICT_ERROR_CHECKING
					throw std::runtime_error("Invalid value for number of building levels: '" + std::string(levelsTag->value) + "'");
#endif
				}

				height = calcBuildingHeightFromLevelCount(levels);
			}
		}

		std::shared_ptr<Building> buildingPtr = std::make_shared<Building>();
		Building& building = *buildingPtr;
		LatLonPoints points;
		readPoints(*way, data, points);

		if (!isClockwise(points))
		{
			std::reverse(points.begin(), points.end());
		}

		if (height > 0.0)
		{
			building.height = height;
		}
		else
		{
			// Estimate height from building size
			LatLonBounds bounds = calcPointBounds(points);
			vis::LatLonVec2Adapter boundsSize = bounds.size();
			double sizeMeters = std::max(boundsSize.x(), boundsSize.y()) * earthRadius;
			int levels = skybolt::math::clamp(int(std::pow(sizeMeters, 0.8) / 10.0f) + 1, 1, 5); // not based on anything, but looks ok
			building.height = calcBuildingHeightFromLevelCount(levels);
		}

		if (points.size() >= 2)
		{
			building.points = toLatLonWithMinAlt(points, *data.altitudeProvider);
			features.push_back(buildingPtr);
		}
	}

	tag = getTag(*way, "natural");
	if (tag)
	{
		if (strcmp(tag->value, "water") == 0)
		{
			LatLonPoints points;
			readPoints(*way, data, points);
			preparePoly(points);

			if (points.size() >= 2)
			{
				std::shared_ptr<Water> waterPtr = std::make_shared<Water>();
				Water& water = *waterPtr;
				water.points = toLatLonAlt(points, *data.altitudeProvider);
				features.push_back(waterPtr);
			}
		}
	}

	tag = getTag(*way, "aeroway");
	if (tag)
	{
		if (strcmp(tag->value, "aerodrome") == 0)
		{
			const char* name = getTagValue(*way, "name");
			if (name)
			{
				std::vector<sim::LatLon> points;
				readPoints(*way, data, points);
				
				ParserData::ParserAirport airport;
				airport.name = name;
				airport.bounds = calcPointBounds(points);
				airport.areaPolygons = { points };
				data.airports.emplace_back(airport);
			}
		} else  if (strcmp(tag->value, "runway") == 0)
		{
			const char* name = getTagValue(*way, "ref");
			if (name)
			{
				std::vector<sim::LatLon> points;
				readPoints(*way, data, points);
				if (!points.empty())
				{
					Airport::Runway runway;
					runway.name = name;
					runway.start = points.front();
					runway.end = points.back();
					runway.width = getTagValueOrDefault(*way, "width", Units::Meters, 45.0);

					data.runways.emplace_back(runway);
				}
			}
		}
	}

	if (data.features.size() % 100000 == 0)
		printf("Loaded %zu features\n", data.features.size());

	return READOSM_OK;
}

std::vector<LatLonPoints> readMultiPolygonRelation(const readosm_relation& relation, const ParserData& data)
{
	std::vector<LatLonPoints> polygons;
	std::vector<std::vector<sim::LatLon>> parts;

	for (int i = 0; i < relation.member_count; ++i)
	{
		const readosm_member& member = relation.members[i];
		if (member.member_type == READOSM_MEMBER_WAY)
		{
			if (strcmp(member.role, "outer") == 0)
			{
				const ParserData::Way& way = data.ways.find(member.id)->second;
				LatLonPoints points;
				readPoints(way, data, points);
				if (points.size() >= 2)
				{
					parts.emplace_back(points);
				}
			}
		}
	}

	if (!parts.empty())
	{
		// OSM ways in a multipolygon are *not* gauranteed to be in contiguous order.
		// We reorder them here into contiguous rings.
		// Two ways join when the last point in one equals the first point in the other.
		std::vector<int> nextPartIndices(parts.size(), -1);
		std::set<int> unallocatedParts;

		for (int i = 0; i < (int)parts.size(); ++i)
		{
			for (int j = 0; j < (int)parts.size(); ++j)
			{
				if (parts[i].back() == parts[j].front())
				{
					nextPartIndices[i] = j;
					break;
				}
			}
			unallocatedParts.insert(i);
		}

		while (!unallocatedParts.empty())
		{
			LatLonPoints points;
			int nextIndex = *unallocatedParts.begin();
			int firstIndex = nextIndex;
			bool joined = false;
			while (nextIndex != -1)
			{
				const LatLonPoints& part = parts[nextIndex];
				points.insert(points.end(), part.begin(), part.begin() + part.size() - 1); // insert from first point to second last point inclusive
				unallocatedParts.erase(nextIndex);
				nextIndex = nextPartIndices[nextIndex];
				if (nextIndex == firstIndex)
				{
					joined = true;
					break;
				}
			}

			if (joined)
			{
				preparePoly(points);
				polygons.push_back(points);
			}
		}
	}

	return polygons;
}

int parseRelation(const void* user_data, const readosm_relation* relation)
{
	ParserData& data = *(ParserData*)user_data;

	if (getTagValueString(*relation, "natural") == "water")
	{
		std::vector<LatLonPoints> polygons = readMultiPolygonRelation(*relation, data);

		std::vector<FeaturePtr>& features = data.features;
		for (const LatLonPoints& polygon : polygons)
		{
			auto water = std::make_shared<Water>();
			water->points = toLatLonAlt(polygon, *data.altitudeProvider);
			features.push_back(water);
		}
	}
	else if (getTagValueString(*relation, "aeroway") == "aerodrome")
	{
		const char* name = getTagValue(*relation, "name");
		if (name)
		{
			std::vector<LatLonPoints> polygons = readMultiPolygonRelation(*relation, data);
			if (!polygons.empty())
			{
				ParserData::ParserAirport airport;
				airport.name = name;
				airport.bounds = calcPointBounds(polygons.front());
				airport.areaPolygons = polygons;
				data.airports.emplace_back(airport);
			}
		}
	}

	return READOSM_OK;
}

// TODO: handle longitude wrap at dateline
double approxDistanceInRadians(const sim::LatLon& a, const sim::LatLon& b)
{
	return std::max(std::fabs(a.lat - b.lat), std::fabs(a.lon - b.lon));
}

// TODO: handle longitude wrap at dateline
sim::LatLon approxAverage(const sim::LatLon& a, const sim::LatLon& b)
{
	return sim::LatLon((a.lat + b.lat) / 2.0, (a.lon + b.lon) / 2.0);
}

const ParserData::ParserAirport* findClosestAirport(const ParserData& data, const sim::LatLon& position)
{
	const ParserData::ParserAirport* result = nullptr;
	double resultDistance = 0.0;
	
	for (const ParserData::ParserAirport& airport : data.airports)
	{
		LatLon boundsSize = airport.bounds.size();
		double airportRadius = std::max(boundsSize.lat, boundsSize.lon); // only accept airports within this radius
		double distance = approxDistanceInRadians(airport.bounds.center(), position) - airportRadius;
		if (distance < resultDistance)
		{
			result = &airport;
			resultDistance = distance;
		}
	}
	return result;
}

std::map<std::string, AirportPtr> createAirports(const ParserData& data, const sim::PlanetAltitudeProvider& provider)
{
	std::map<const ParserData::ParserAirport*, AirportPtr> airports;
	for (const auto& v : data.airports)
	{
		auto airport = std::make_shared<Airport>();
		airport->areaPolygons = v.areaPolygons;
		airport->altitude = provider.getAltitude(v.bounds.center()).altitude;
		airports[&v] = airport;
	}

	// Add runways
	for (const auto& runway : data.runways)
	{
		// Add to closest airport
		const ParserData::ParserAirport* airport = findClosestAirport(data, approxAverage(runway.start, runway.end));
		if (airport)
		{
			airports[airport]->runways.push_back(runway);
		}
	}

	// Return airports with runways
	std::map<std::string, AirportPtr> result;
	for (const auto& v : airports)
	{
		if (!v.second->runways.empty())
		{
			result[v.first->name] = v.second;
		}
	}
	return result;
}

static void joinRoadsAtJunctions(const ParserData& data)
{
	for (const auto& [node, junction] : data.nodeRoadJunctions)
	{
		joinRoadsAtJunction(junction);
	}
}

ReadPbfResult readPbf(const std::string& filename, const sim::PlanetAltitudeProvider& provider)
{
	ParserData data;
	data.altitudeProvider = &provider;
	const void *osm_handle;
	try
	{
		int ret = readosm_open(filename.c_str(), &osm_handle);

		if (ret != READOSM_OK)
		{
			throw skybolt::Exception("Could not open file");
		}

		const void *userData = &data;
		ret = readosm_parse(osm_handle, userData, parseNode, parseWay, parseRelation);
		if (ret != READOSM_OK)
		{
			std::stringstream ss;
			ss << ret;
			throw skybolt::Exception("Error parsing file. Error code: " + ss.str());
		}
	}
	catch(const std::exception& e)
	{
		readosm_close(osm_handle);
		throw skybolt::Exception("Error converting " + filename + ". Reason: " + e.what());
	}
	readosm_close(osm_handle);

	printf("Connecting roads at %zu connection points\n", data.nodeRoadJunctions.size());
	joinRoadsAtJunctions(data);

	ReadPbfResult result;
	printf("Matching %zu runways with %zu airports\n", data.runways.size(), data.airports.size());
	std::swap(result.features, data.features);
	result.airports = createAirports(data, provider);
	for (const auto& v : result.airports)
	{
		result.features.push_back(v.second);
	}

	return result;
}

} // namespace mapfeatures
} // namespace skybolt