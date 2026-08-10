/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightmapLeveler.h"
#include <SkyboltCommon/Math/Box2.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltVis/GeoImageHelpers.h>
#include <SkyboltVis/Image/SimpleImageFactory.h>

#include <filesystem>


namespace skybolt {
namespace mapfeatures {

using vis::Image;
using vis::ImagePtr;
using vis::ImageFactory;

//! @return inclusive bounds
static Box2i toIntBounds(const Image& image, const Box2d& bounds)
{
	Box2i r;
	r.minimum.x = std::min((int)bounds.minimum.x, image.getWidth() - 1);
	r.maximum.x = std::min((int)bounds.maximum.x, image.getWidth() - 1);
	r.minimum.y = std::min((int)bounds.minimum.y, image.getHeight() - 1);
	r.maximum.y = std::min((int)bounds.maximum.y, image.getHeight() - 1);
	return r;
}

static Box2f toBox2f(const LatLonBounds& b)
{
	return Box2f(glm::vec2(b.minimum.x(), b.minimum.y()), glm::vec2(b.maximum.x(), b.maximum.y()));
}

static void fillSubImage(Image& image, const Box2i& bounds, uint16_t v)
{
	uint16_t* p = reinterpret_cast<uint16_t*>(image.getRawData());
	for (int y = bounds.minimum.y; y <= bounds.maximum.y; ++y)
	{
		for (int x = bounds.minimum.x; x <= bounds.maximum.x; ++x)
		{
			p[x + image.getWidth() * y] = v;
		}
	}
}

static uint16_t calcMeanHeight(const Image& image, const Box2i& bounds)
{
	const uint16_t* p = reinterpret_cast<const uint16_t*>(image.getRawData());
	double mean = 0;

	for (int y = bounds.minimum.y; y <= bounds.maximum.y; ++y)
	{
		for (int x = bounds.minimum.x; x <= bounds.maximum.x; ++x)
		{
			mean += p[x + image.getWidth() * y];
		}
	}

	mean /= (bounds.maximum.x - bounds.minimum.x + 1) * (bounds.maximum.y - bounds.minimum.y + 1);
	return (uint16_t)mean;
}

static Box2d toBox2d(const LatLonBounds& boudns)
{
	return Box2d(glm::dvec2(boudns.minimum.x(), boudns.minimum.y()), glm::dvec2(boudns.maximum.x(), boudns.maximum.y()));
}

static Box2i getSubImageBounds(const Image& image, const LatLonBounds& imageWorldBounds, const LatLonBounds& featureBounds)
{
	Box2d subImageBoundsF = vis::getSubImageBounds(toBox2d(imageWorldBounds), toBox2d(featureBounds), image.getWidth(), image.getHeight());
	Box2d flippedSubImageBoundsF(
		glm::dvec2(subImageBoundsF.minimum.y, subImageBoundsF.minimum.x),
		glm::dvec2(subImageBoundsF.maximum.y, subImageBoundsF.maximum.x)
	);

	return toIntBounds(image, flippedSubImageBoundsF);
}

static std::string getDir(const skybolt::QuadTreeTileKey& key)
{
	return std::to_string(key.level) + "/" + std::to_string(key.x);
}

static std::string getFilename(const skybolt::QuadTreeTileKey& key)
{
	return getDir(key) + "/" + std::to_string(key.y) + ".png";
}

typedef std::map<skybolt::QuadTreeTileKey, mapfeatures::LatLonBounds> KeyBounds;

static LatLonBounds calcWorldBounds(const Feature& feature, double borderMeters)
{
	LatLonBounds bounds = feature.calcBounds();
	double borderRadians = borderMeters / sim::earthRadius();
	bounds.minimum.lat -= borderRadians;
	bounds.minimum.lon -= borderRadians;
	bounds.maximum.lat += borderRadians;
	bounds.maximum.lon += borderRadians;
	return bounds;
}

KeyBounds findTilesIntersectingBounds(const std::string& heightmapSourceDirectory, const LatLonBounds& bounds)
{
	KeyBounds keyBounds;

	namespace fs = std::filesystem;

	std::vector<const FeatureTile*> tiles;
	auto subdivisionRequiredPredicate = [&](const FeatureTile& tile) {

		skybolt::QuadTreeTileKey key = tile.key;

		std::string filename = getFilename(key);
		std::string sourceFilepath = heightmapSourceDirectory + "/" + filename;
		if (!fs::exists(sourceFilepath))
			return false;

		bool intersects = tile.bounds.intersects(bounds);
		if (intersects)
		{
			keyBounds[key] = tile.bounds;
		}
		return intersects;
	};

	WorldFeatures features;
	features.tree.leftTree.subdivideRecursively(features.tree.leftTree.getRoot(), subdivisionRequiredPredicate);
	features.tree.rightTree.subdivideRecursively(features.tree.rightTree.getRoot(), subdivisionRequiredPredicate);

	return keyBounds;
}

struct TileInfo
{
	std::vector<Feature*> features;
	LatLonBounds worldBounds;
};

typedef std::map<int, uint16_t> LevelElevations;

struct FeatureInfo
{
	LevelElevations elevations;
	LatLonBounds worldBounds;
};

void levelHeightmapUnderFeatures(const skybolt::QuadTreeTileKey& key, const TileInfo& tileInfo, Image& image, std::map<Feature*, FeatureInfo>& featureInfoMap)
{
	for (Feature* feature : tileInfo.features)
	{
		FeatureInfo& info = featureInfoMap.find(feature)->second;
		Box2i subImageBounds = getSubImageBounds(image, tileInfo.worldBounds, info.worldBounds);

		// Cache elevations by tile level to ensure all tiles use the same elevation so that edges match up
		uint16_t elevation;
		auto it = info.elevations.find(key.level);
		if (it == info.elevations.end())
		{
			elevation = calcMeanHeight(image, subImageBounds);
			info.elevations[key.level] = elevation;
		}
		else
		{
			elevation = it->second;
		}

		fillSubImage(image, subImageBounds, elevation);
	}
}

typedef std::map<skybolt::QuadTreeTileKey, TileInfo> TileInfoForKeys;

void levelHeightmapsUnderFeatures(const ImageFactory& imageFactory, const std::string& heightmapSourceDirectory, const std::string& heightmapDestinationDirectory, const std::vector<Feature*>& features, double borderMeters)
{
	// For each tile, find the intersecting features
	TileInfoForKeys tiles;
	std::map<Feature*, FeatureInfo> featureInfoMap;

	for (Feature* feature : features)
	{
		LatLonBounds featureWorldBounds = calcWorldBounds(*feature, borderMeters);
		featureInfoMap[feature].worldBounds = featureWorldBounds;

		KeyBounds keyBounds = findTilesIntersectingBounds(heightmapSourceDirectory, featureWorldBounds);

		for (const auto& v : keyBounds)
		{
			auto& info = tiles[v.first];
			info.worldBounds = v.second;
			info.features.push_back(feature);
		}
	}

	// Level heightmaps under features
	for (const auto& v : tiles)
	{
		std::string filename = getFilename(v.first);
		std::string sourceFilepath = heightmapSourceDirectory + "/" + filename;
		ImagePtr image = valueOrThrowException(imageFactory.readImage(sourceFilepath));

		levelHeightmapUnderFeatures(v.first, v.second, *image, featureInfoMap);

		std::filesystem::create_directories(heightmapDestinationDirectory + "/" + getDir(v.first));
		valueOrThrowException(imageFactory.writeImage(*image, heightmapDestinationDirectory + "/" + filename));
	}
}

static float heightmapValueToFloatAltitude(float value)
{
	const float heightMapSeaLevelValue = 32768;
	return value;
}

double getAltitudeAtPosition(const ImageFactory& imageFactory, const std::string& heightmapSourceDirectory, const sim::LatLon& position)
{
	LatLonBounds positionBounds(position, position);
	KeyBounds keyBounds = findTilesIntersectingBounds(heightmapSourceDirectory, positionBounds);

	if (keyBounds.empty())
	{
		return 0;
	}

	int highestLevel = -1;
	skybolt::QuadTreeTileKey key;
	LatLonBounds tileBounds;

	for (const auto& v : keyBounds)
	{
		int level = v.first.level;
		if (level > highestLevel)
		{
			highestLevel = level;
			key = v.first;
			tileBounds = v.second;
		}
	}

	std::string filename = getFilename(key);
	std::string sourceFilepath = heightmapSourceDirectory + "/" + filename;
	ImagePtr image = valueOrThrowException(imageFactory.readImage(sourceFilepath));

	Box2i subImageBounds = getSubImageBounds(*image, tileBounds, positionBounds);
	uint16_t height = calcMeanHeight(*image, subImageBounds);
	return heightmapValueToFloatAltitude(height);
}

} // namespace mapfeatures
} // namespace skybolt