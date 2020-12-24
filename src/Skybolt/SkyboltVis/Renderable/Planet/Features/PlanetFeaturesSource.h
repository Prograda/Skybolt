/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h"
#include <SkyboltSim/Spatial/LatLonAlt.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osg/Vec2d>

#include <assert.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace skybolt {
namespace mapfeatures {

typedef skybolt::Box2T<vis::LatLonVec2Adapter> LatLonBounds;

LatLonBounds calcPointBounds(const std::vector<sim::LatLon>& points);

enum FeatureType
{
	FeatureRoad,
	FeatureBuilding,
	FeatureWater,
	FeatureAirport,
	FeatureMax
};

struct Feature
{
	virtual ~Feature() {}
	virtual FeatureType type() const = 0;

	virtual void load(std::ifstream& f) = 0;
	virtual void save(std::ofstream& f) const = 0;
	virtual LatLonBounds calcBounds() const = 0;
};

struct PolyFeature : public Feature
{
	std::vector<sim::LatLonAlt> points;

	void load(std::ifstream& f) override;
	void save(std::ofstream& f) const override;
	LatLonBounds calcBounds() const override;
};

struct Road : public PolyFeature
{
	FeatureType type() const override { return FeatureRoad; }

	float width;
	int laneCount;
	
	void load(std::ifstream& f) override;
	void save(std::ofstream& f) const override;
};

struct Building : public PolyFeature
{
	FeatureType type() const override { return FeatureBuilding; }

	float height;
	
	void load(std::ifstream& f) override;
	void save(std::ofstream& f) const override;
};

struct Water : public PolyFeature
{
	FeatureType type() const override { return FeatureWater; }
};

typedef std::vector<sim::LatLon> LatLonPoints;
typedef std::vector<sim::LatLonAlt> LatLonAltPoints;

struct Airport : public Feature
{
	FeatureType type() const override { return FeatureAirport; }

	struct Runway
	{
		std::string name;
		sim::LatLon start;
		sim::LatLon end;
		float width;
	};

	std::vector<Runway> runways;
	std::vector<LatLonPoints> areaPolygons; //!< Polygons that define the airport area
	double altitude = 0;

	void load(std::ifstream& f) override;
	void save(std::ofstream& f) const override;
	LatLonBounds calcBounds() const override;
};

typedef std::shared_ptr<Feature> FeaturePtr;
typedef std::shared_ptr<Airport> AirportPtr;

struct FeatureTile : public skybolt::QuadTreeTile<vis::LatLonVec2Adapter, FeatureTile>
{
	std::vector<FeaturePtr> features;
	size_t featureCountInFile = 0;
};

struct WorldFeatures
{
	typedef skybolt::QuadTree<FeatureTile> QuadTree;
	typedef skybolt::DiQuadTree<FeatureTile> DiQuadTree;
	DiQuadTree tree;

	WorldFeatures(WorldFeatures::QuadTree::TileCreator tileCreator = createTile);

	static std::unique_ptr<FeatureTile> createTile(const skybolt::QuadTreeTileKey& key, const LatLonBounds& bounds);
};

struct TreeCreatorParams
{
	double minFeatureSizeFraction; //!< Max size of a feature in a tile as a fraction of the tile size
	int maxLodLevel;
};

WorldFeatures createWorldFeatures(const TreeCreatorParams& params, const std::vector<FeaturePtr>& features);

void saveTile(const FeatureTile& tile, const std::string& filename);
void loadTile(const std::string& filename, std::vector<FeaturePtr>& features);

void save(const WorldFeatures::DiQuadTree& tree, const std::string& directory);
void loadJsonFromDirectory(WorldFeatures& features, const std::string& directory);

void saveAirports(const std::map<std::string, AirportPtr>& airports, const std::string& filename);
std::map<std::string, AirportPtr> loadAirports(const std::string& filename);

std::string getTilePathFromKey(const skybolt::QuadTreeTileKey& key);

std::string statsToString(const std::vector<FeaturePtr>& features);

} // namespace mapfeatures
} // namespace skybolt