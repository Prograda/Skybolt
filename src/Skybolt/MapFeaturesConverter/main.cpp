/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FeaturesConverter.h"

#define SAVE_LEVELED_HEIGHTMAPS
#ifdef SAVE_LEVELED_HEIGHTMAPS
#include "HeightmapLeveler.h"
#endif

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h>


using namespace skybolt;
using namespace mapfeatures;
using namespace vis;

int main()
{
	int maxLod = 10;
	std::string directory = "Assets/Core/Tiles/Earth/Features";

	XyzTileSourceConfig config;
	config.urlTemplate = "DEM/CombinedElevation/{z}/{x}/{y}.png";

	auto tileSource = std::make_shared<XyzTileSource>(config);
	TilePlanetAltitudeProvider altitudeProvider(tileSource, maxLod);
	ReadPbfResult result = mapfeatures::readPbf("washington-latest.osm.pbf", altitudeProvider);
	{
		printf("Feature Conversion Stats:\n%s\n", mapfeatures::statsToString(result.features).c_str());

		mapfeatures::TreeCreatorParams treeCreatorParams;
		treeCreatorParams.minFeatureSizeFraction = 0.1;
		treeCreatorParams.maxLodLevel = maxLod;
		mapfeatures::WorldFeatures worldFeatures = mapfeatures::createWorldFeatures(treeCreatorParams, result.features);

#ifdef SAVE_LEVELED_HEIGHTMAPS
		std::string heightmapSourceDirectory = "DEM/CombinedElevation";
		std::string heightmapDestinationDirectory = "Assets/Core/Tiles/Earth/Elevation";
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
		mapfeatures::save(worldFeatures.tree, directory);
		mapfeatures::saveAirports(result.airports, "Assets/Core/Airports/airports.apt");
	}
}