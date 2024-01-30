/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FeaturesConverter.h"
#include <iostream>

//#define PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
#ifdef PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
#include "HeightmapLeveler.h"
#endif

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/CachedTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/MapboxElevationTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/SphericalMercatorToPlateCarreeTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/ShaUtility.h>

using namespace skybolt;
using namespace mapfeatures;
using namespace vis;

int maxFeatureTileLod = 10;
int maxHeightmapTileLod = 12;
std::string outputDirectory = "Output";
std::string tileCacheDirectory = "cache";
std::string heightmapSourceDirectory = "DEM/CombinedElevation";
std::string heightmapDestinationDirectory = "SkyboltAssets/Assets/SeattleElevation/Tiles/Earth/Elevation";

int main(int argc, char *argv[])
{
	try
	{
		auto params = EngineCommandLineParser::parse(argc, argv);
		nlohmann::json settings = readEngineSettings(params);
		auto tileApiKeys = readNameMap<std::string>(settings, "tileApiKeys");

#define USE_DEM
#ifdef USE_DEM
		XyzTileSourceConfig config;
		config.urlTemplate = "DEM/{z}/{x}/{y}.png";
		config.elevationRerange = rerangeElevationFromUInt16WithElevationBounds(-32768, 32767);
		config.levelRange = {0, 10};

		auto tileSource = std::make_shared<XyzTileSource>(config);
#else // use mapbox
		auto mapboxKey = findOptional<std::string>(tileApiKeys, "mapbox");
		if (!mapboxKey)
		{
			throw std::runtime_error("Mapbox key not found in Skybolt settings file");
		}

		MapboxElevationTileSourceConfig config;
		config.urlTemplate = "http://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token={key}";
		config.apiKey = *mapboxKey;

		auto uncachedTileSource = std::make_shared<SphericalMercatorToPlateCarreeTileSource>(std::make_shared<MapboxElevationTileSource>(config));
		std::string tileSourceCacheDirectory = tileCacheDirectory  + "/" + calcSha1(config.urlTemplate);
		auto tileSource = std::make_shared<CachedTileSource>(uncachedTileSource, tileSourceCacheDirectory);
#endif
		BlockingTilePlanetAltitudeProvider altitudeProvider(tileSource, maxHeightmapTileLod);
		ReadPbfResult result = mapfeatures::readPbf("washington-latest.osm.pbf", altitudeProvider);
		{
			printf("Feature Conversion Stats:\n%s\n", mapfeatures::statsToString(result.features).c_str());

			mapfeatures::TreeCreatorParams treeCreatorParams;
			treeCreatorParams.minFeatureSizeFraction = 0.1;
			treeCreatorParams.maxLodLevel = maxFeatureTileLod;
			mapfeatures::WorldFeatures worldFeatures = mapfeatures::createWorldFeatures(treeCreatorParams, result.features);

#ifdef PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
			double borderMeters = 100.0;
			std::vector<Feature*> airportFeatures;
			for (const auto& a : result.airports)
			{
				airportFeatures.push_back(a.second.get());
			}
			mapfeatures::levelHeightmapsUnderFeatures(heightmapSourceDirectory, heightmapDestinationDirectory, airportFeatures, borderMeters);

			for (const auto& airport : airportFeatures)
			{
				static_cast<Airport*>(airport)->altitude = mapfeatures::getAltitudeAtPosition(heightmapDestinationDirectory, airport->calcBounds().center());
			}
#endif
			mapfeatures::save(worldFeatures.tree, outputDirectory);
			mapfeatures::saveAirports(result.airports, outputDirectory + "/airports.apt");
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception thrown: " << e.what() << std::endl;
	}
}