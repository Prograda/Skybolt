/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetFeaturesSource.h"
#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <limits>
#include <iostream>
#include <iomanip>
#include <sstream>

using skybolt::Exception;
using namespace skybolt::math;
using skybolt::QuadTreeTileKey;
using skybolt::sim::LatLon;
using skybolt::sim::LatLonAlt;

namespace skybolt {
namespace mapfeatures {

LatLonBounds calcPointBounds(const std::vector<sim::LatLon>& points)
{
	LatLonBounds bounds;

	for (size_t i = 0; i < points.size(); ++i)
	{
		bounds.merge(points[i]);
	}

	return bounds;
}

static LatLonBounds calcPointBounds(const std::vector<sim::LatLonAlt>& points)
{
	LatLonBounds bounds;

	for (size_t i = 0; i < points.size(); ++i)
	{
		bounds.merge(toLatLon(points[i]));
	}

	return bounds;
}

template <typename T>
void readValue(std::ifstream& f, T& value)
{
	f.read((char*)&value, sizeof(value));
}

template <typename T>
void writeValue(std::ofstream& f, const T& value)
{
	f.write((const char*)&value, sizeof(value));
}

static void readLatLon(std::ifstream& f, sim::LatLon& latLon)
{
	readValue(f, latLon.lat);
	readValue(f, latLon.lon);
}

static void writeLatLon(std::ofstream& f, const sim::LatLon& latLon)
{
	writeValue(f, latLon.lat);
	writeValue(f, latLon.lon); 
}

static void readLatLonAlt(std::ifstream& f, sim::LatLonAlt& latLonAlt)
{
	readValue(f, latLonAlt.lat);
	readValue(f, latLonAlt.lon);
	readValue(f, latLonAlt.alt);
}

static void writeLatLonAlt(std::ofstream& f, const sim::LatLonAlt& latLonAlt)
{
	writeValue(f, latLonAlt.lat);
	writeValue(f, latLonAlt.lon);
	writeValue(f, latLonAlt.alt);
}

static void readPoints(std::ifstream& f, LatLonPoints& points)
{
	int pointCount;
	readValue(f, pointCount);

	if (pointCount)
	{
		points.resize(pointCount);
		for (LatLon& p : points)
		{
			readLatLon(f, p);
		}
	}
}

static void writePoints(std::ofstream& f, const LatLonPoints& points)
{
	int pointCount = (int)points.size();
	writeValue(f, pointCount);
	for (const LatLon& p : points)
	{
		writeLatLon(f, p);
	}
}

static void readPoints(std::ifstream& f, LatLonAltPoints& points)
{
	int pointCount;
	readValue(f, pointCount);

	if (pointCount)
	{
		points.resize(pointCount);
		for (LatLonAlt& p : points)
		{
			readLatLonAlt(f, p);
		}
	}
}

static void writePoints(std::ofstream& f, const LatLonAltPoints& points)
{
	int pointCount = (int)points.size();
	writeValue(f, pointCount);
	for (const LatLonAlt& p : points)
	{
		writeLatLonAlt(f, p);
	}
}

void PolyFeature::load(std::ifstream& f)
{
	readPoints(f, points);
}

void PolyFeature::save(std::ofstream& f) const
{
	writePoints(f, points);
}

LatLonBounds PolyFeature::calcBounds() const
{
	return calcPointBounds(points);
}

void Road::load(std::ifstream& f)
{
	readValue(f, width);
	readValue(f, laneCount);
	PolyFeature::load(f);
}

void Road::save(std::ofstream& f) const
{
	writeValue(f, width);
	writeValue(f, laneCount);
	PolyFeature::save(f);
}

void Building::load(std::ifstream& f)
{
	readValue(f, height);
	PolyFeature::load(f);
}

void Building::save(std::ofstream& f) const
{
	writeValue(f, height);
	PolyFeature::save(f);
}

static std::string readString(std::ifstream& f)
{
	std::string str;
	uint16_t size;
	readValue(f, size);
	str.resize(size);
	f.read(&str[0], size);
	return str;
}

static void writeString(std::ofstream& f, const std::string& str)
{
	uint16_t size = str.size();
	writeValue(f, size);
	f.write(&str[0], size);
}

static void readRunway(std::ifstream& f, Airport::Runway& runway)
{
	runway.name = readString(f);
	readLatLon(f, runway.start);
	readLatLon(f, runway.end);
	readValue(f, runway.width);
}

static void writeRunway(std::ofstream& f, const Airport::Runway& runway)
{
	writeString(f, runway.name);
	writeLatLon(f, runway.start);
	writeLatLon(f, runway.end);
	writeValue(f, runway.width);
}

static void readPolygons(std::ifstream& f, std::vector<LatLonPoints>& polygons)
{
	uint16_t areaPolygonCount;
	readValue(f, areaPolygonCount);
	polygons.resize(areaPolygonCount);
	for (uint16_t i = 0; i < areaPolygonCount; ++i)
	{
		readPoints(f, polygons[i]);
	}
}

static void writePolygons(std::ofstream& f, const std::vector<LatLonPoints>& polygons)
{
	uint16_t areaPolygonCount = polygons.size();
	writeValue(f, areaPolygonCount);
	for (uint16_t i = 0; i < areaPolygonCount; ++i)
	{
		writePoints(f, polygons[i]);
	}
}

void Airport::load(std::ifstream& f)
{
	{
		uint16_t runwayCount;
		readValue(f, runwayCount);
		runways.resize(runwayCount);
		for (uint16_t i = 0; i < runwayCount; ++i)
		{
			readRunway(f, runways[i]);
		}
	}
	readPolygons(f, areaPolygons);
	readValue(f, altitude);
}

void Airport::save(std::ofstream& f) const
{
	uint16_t runwayCount = runways.size();
	writeValue(f, runwayCount);

	for (uint16_t i = 0; i < runways.size(); ++i)
	{
		writeRunway(f, runways[i]);
	}
	writePolygons(f, areaPolygons);
	writeValue(f, altitude);
}

LatLonBounds Airport::calcBounds() const
{
	LatLonBounds bounds;
	for (const Runway& runway : runways)
	{
		bounds.merge(runway.start);
		bounds.merge(runway.end);
	}
	for (const LatLonPoints& points : areaPolygons)
	{
		for (const sim::LatLon& point : points)
		{
			bounds.merge(point);
		}
	}
	return bounds;
}

static FeaturePtr createFeature(FeatureType type)
{
	switch(type)
	{
	case FeatureRoad:
		return std::make_shared<Road>();
	case FeatureBuilding:
		return std::make_shared<Building>();
	case FeatureWater:
		return std::make_shared<Water>();
	case FeatureAirport:
		return std::make_shared<Airport>();
	}
	assert(!"Not implemented");
	return nullptr;
}

static void load(std::ifstream& f, std::vector<FeaturePtr>& features)
{
	uint32_t typeCount;
	readValue(f, typeCount);
	for (uint32_t i = 0; i < typeCount; ++i)
	{
		uint32_t type;
		readValue(f, type);

		size_t featureCount;
		readValue(f, featureCount);
		
		for (size_t j = 0; j < featureCount; ++j)
		{
			FeaturePtr feature = createFeature((FeatureType)type);
			feature->load(f);
			features.push_back(feature);
		}
	}
}

typedef std::map<FeatureType, size_t> FeatureTypeCounts;

static FeatureTypeCounts countFeatureTypes(const std::vector<FeaturePtr>& features)
{
	FeatureTypeCounts counts;
	for (const FeaturePtr& feature : features)
	{
		++counts[feature->type()];
	}
	return counts;
}

static void save(std::ofstream& f, const std::vector<FeaturePtr>& features)
{
	FeatureTypeCounts counts = countFeatureTypes(features);
	uint32_t countsSize = counts.size();

	writeValue(f, countsSize);

	for (const auto& entry : counts)
	{
		uint32_t type = entry.first;
		writeValue(f, type);
		writeValue(f, entry.second);
		
		for (const FeaturePtr& feature : features)
		{
			if (feature->type() == type)
			{
				feature->save(f);
			}
		}
	}
}

static const uint32_t fileVersion = 1;

void loadTile(const std::string& filename, std::vector<FeaturePtr>& features)
{
	std::ifstream f(filename, std::ios::binary);

	if (!f.is_open())
	{
		throw Exception("Could not open file: " + filename);
	}

	uint32_t version;
	f.read((char*)&version, sizeof(uint32_t));

	if (version != fileVersion)
	{
		throw Exception("Invalid file version: " + std::to_string(version) + ". Expected: " + std::to_string(fileVersion));
	}

	load(f, features);
}

void saveTile(const FeatureTile& tile, const std::string& filename)
{
	std::ofstream f(filename, std::ios::binary);

	// Write file version
	int version = fileVersion;
	writeValue(f, version);

	// Write features
	save(f, tile.features);

	f.close();
}

static nlohmann::json tileToJsonRecursive(const FeatureTile& tile)
{
	nlohmann::json j;
	j["level"] = tile.key.level;
	j["x"] = tile.key.x;
	j["y"] = tile.key.y;
	j["featureCount"] = tile.features.size();

	if (tile.hasChildren())
	{
		std::vector<nlohmann::json> children;
		for (int i = 0; i < 4; ++i)
		{
			children.push_back(tileToJsonRecursive(*tile.children[i]));
		}
		j["children"] = children;
	}
	return j;
}

static void addJsonTilesToTreeRecursive(FeatureTile& tile, WorldFeatures::QuadTree& tree, const nlohmann::json& j)
{
	tile.key.level = j["level"];
	tile.key.x = j["x"];
	tile.key.y = j["y"];
	tile.featureCountInFile += j["featureCount"];
	
	auto it = j.find("children");
	if (it != j.end())
	{
		const std::vector<nlohmann::json>& children = *it;
		if (!children.empty())
		{
			if (children.size() != 4)
			{
				throw skybolt::Exception("Tile in tree does not have 4 children. Found " + std::to_string(j.size()));
			}
			else
			{
				if (!tile.hasChildren())
				{
					tree.subdivide(tile);
				}
				for (int i = 0; i < children.size(); ++i)
				{
					addJsonTilesToTreeRecursive(*tile.children[i], tree, children[i]);
				}
			}
		}
	}
}

std::string getTilePathFromKey(const QuadTreeTileKey& key)
{
	return std::to_string(key.level) + "/" + std::to_string(key.x) + "/" + std::to_string(key.y) + ".ftr";
}

static void saveTileRecursive(const FeatureTile& tile, const std::string& directory)
{
	if (!tile.features.empty())
	{
		std::string tileDirectory = directory + "/" + std::to_string(tile.key.level) + "/" + std::to_string(tile.key.x);
		std::filesystem::create_directories(tileDirectory);
		saveTile(tile, tileDirectory + "/" + std::to_string(tile.key.y) + ".ftr");
	}

	if (tile.hasChildren())
	{
		for (int i = 0; i < 4; ++i)
		{
			saveTileRecursive(*tile.children[i], directory);
		}
	}
}

static const std::string treeFilename = "tree.json";

void save(const WorldFeatures::DiQuadTree& tree, const std::string& directory)
{
	std::filesystem::create_directories(directory);
	std::ofstream f(directory + "/" + treeFilename, std::ios::out | std::ios::binary);

	nlohmann::json j;
	j["leftRoot"] = tileToJsonRecursive(tree.leftTree.getRoot());
	j["rightRoot"] = tileToJsonRecursive(tree.rightTree.getRoot());

	f << std::setw(1) << j;

	f.close();

	saveTileRecursive(tree.leftTree.getRoot(), directory);
	saveTileRecursive(tree.rightTree.getRoot(), directory);
}

void addJsonFileTilesToTree(WorldFeatures& features, const std::string& filename)
{
	WorldFeatures::DiQuadTree& tree = features.tree;

	std::ifstream f(filename, std::ios::in | std::ios::binary);
	if (f.is_open())
	{
		nlohmann::json j;
		f >> j;
		f.close();

		addJsonTilesToTreeRecursive(tree.leftTree.getRoot(), tree.leftTree, j["leftRoot"]);
		addJsonTilesToTreeRecursive(tree.rightTree.getRoot(), tree.rightTree, j["rightRoot"]);
	}
	else
	{
		throw skybolt::Exception("Could not load WorldFeatures tree from file: " + filename);
	}
}

struct BoundedFeature
{
	FeaturePtr feature;
	LatLonBounds bounds;
};

static double maxSize(const LatLonBounds& box)
{
	vis::LatLonVec2Adapter size = box.size();
	return std::max(size.x(), size.y());
}

static void addToTile(WorldFeatures::QuadTree& tree, FeatureTile& tile, const TreeCreatorParams& params, const BoundedFeature& feature)
{
	if (tile.bounds.intersects(feature.bounds.center()))
	{
		double minAllowedSize = maxSize(tile.bounds) * params.minFeatureSizeFraction;

		if (tile.key.level <= params.maxLodLevel && maxSize(feature.bounds) < minAllowedSize)
		{
			if (!tile.hasChildren())
			{
				tree.subdivide(tile);
			}
			for (int i = 0; i < 4; ++i)
			{
				addToTile(tree, *tile.children[i], params, feature);
			}
		}
		else
		{
			tile.features.push_back(feature.feature);
		}
	}
}

WorldFeatures::WorldFeatures(WorldFeatures::QuadTree::TileCreator tileCreator) :
	tree(createGlobeQuadTree<FeatureTile>(tileCreator))
{
}

std::unique_ptr<FeatureTile> WorldFeatures::createTile(const QuadTreeTileKey& key, const LatLonBounds& bounds)
{
	FeatureTile* tile = new FeatureTile;
	tile->key = key;
	tile->bounds = bounds;
	return std::unique_ptr<FeatureTile>(tile);
};

WorldFeatures createWorldFeatures(const TreeCreatorParams& params, const std::vector<FeaturePtr>& features)
{
	WorldFeatures worldFeatures;
	for (const FeaturePtr& feature : features)
	{
		BoundedFeature boundedFeature;
		boundedFeature.feature = feature;
		boundedFeature.bounds = feature->calcBounds();
		addToTile(worldFeatures.tree.leftTree, worldFeatures.tree.leftTree.getRoot(), params, boundedFeature);
		addToTile(worldFeatures.tree.rightTree, worldFeatures.tree.rightTree.getRoot(), params, boundedFeature);
	}
	
	return worldFeatures;
}

std::map<std::string, AirportPtr> loadAirports(const std::string& filename)
{
	std::map<std::string, AirportPtr> result;

	std::ifstream f(filename, std::ios::binary);
	if (f.is_open())
	{
		size_t size;
		f.read((char*)&size, sizeof(size_t));
		
		for (size_t i = 0; i < size; ++i)
		{
			AirportPtr airport = std::make_shared<Airport>();
			std::string name = readString(f);
			airport->load(f);
			result[name] = airport;
		}

		f.close();
	}
	else
	{
		throw skybolt::Exception("Could not load airports file '" + filename + "'");
	}

	return result;
}

void saveAirports(const std::map<std::string, AirportPtr>& airports, const std::string& filename)
{
	std::ofstream f(filename, std::ios::binary);
	size_t size = airports.size();
	f.write((const char*)&size, sizeof(size_t));
	for (const auto& v : airports)
	{
		writeString(f, v.first);
		v.second->save(f);
	}
	f.close();
}

std::string statsToString(const std::vector<FeaturePtr>& features)
{
	std::map<FeatureType, int> counts;
	for (const auto& feature : features)
	{
		++counts[feature->type()];
	}
	std::ostringstream ss;
	ss << "Roads: " << counts[FeatureRoad]
		<< ", Buildings: " << counts[FeatureBuilding]
		<< ", Water: " << counts[FeatureWater]
		<< ", Airports: " << counts[FeatureAirport];
	return ss.str();
}

} // namespace mapfeatures
} // namespace skybolt