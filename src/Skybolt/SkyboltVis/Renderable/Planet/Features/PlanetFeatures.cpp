/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetFeatures.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include "SkyboltVis/LlaToNedConverter.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Renderable/BuildingsBatch.h"
#include "SkyboltVis/Renderable/RoadsBatch.h"
#include "SkyboltVis/Renderable/RunwaysBatch.h"
#include "SkyboltVis/Renderable/Water/LakesBatch.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"

#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Math/QuadTreeUtility.h>

#include <cxxtimer/cxxtimer.hpp>
#include <boost/algorithm/string.hpp>
#include <mutex>

using namespace skybolt;
using namespace mapfeatures;

namespace skybolt {
namespace vis {

struct LoadedVisObjects
{
	std::vector<VisObjectPtr> nodes[PlanetFeaturesParams::featureGroupsSize] = {};
	sim::LatLon latLonOrigin;
};

class VisObjectsLoadTask
{
public:
	VisObjectsLoadTask(const ShaderPrograms* programs, const osg::ref_ptr<osg::StateSet>& waterStateSet, const BuildingTypesPtr& buildingTypes) :
		mPrograms(programs),
		mWaterStateSet(waterStateSet),
		mBuildingTypes(buildingTypes)
	{
		assert(mPrograms);
		assert(mBuildingTypes);
	}

	//! May be called on multiple threads concurrently
	std::unique_ptr<LoadedVisObjects> loadVisObjects(const std::vector<mapfeatures::FeaturePtr>& features, const sim::LatLon& latLonOrigin, double planetRadius) const
	{
		std::unique_ptr<LoadedVisObjects> objectsPtr = std::make_unique<LoadedVisObjects>();
		LoadedVisObjects& objects = *objectsPtr;
		objects.latLonOrigin = latLonOrigin;

		LlaToNedConverterPtr nedConverter(new LlaToNedConverter(latLonOrigin, planetRadius));
		const LlaToNedConverter& converter = *nedConverter;

		Roads roads;
		Runways runways;
		Buildings buildings;
		Lakes lakes;
		PolyRegions polyRegions;

		for (const mapfeatures::FeaturePtr& feature : features)
		{
			switch (feature->type())
			{
			case mapfeatures::FeatureRoad:
			{
				const mapfeatures::Road& srcRoad = static_cast<const mapfeatures::Road&>(*feature);

				Road road;
				for (int j = 0; j < srcRoad.points.size(); ++j)
				{
					osg::Vec3f pos = converter.latLonAltToCartesianNed(srcRoad.points[j]);
					road.points.push_back(pos);
				}
				road.width = srcRoad.width;
				road.laneCount = srcRoad.laneCount;
				
				for (int i = 0; i < 2; ++i)
				{
					road.endLaneCounts[i] = srcRoad.endLaneCounts[i];
					if (road.endLaneCounts[i] != -1)
					{
						road.endControlPoints[i] = converter.latLonAltToCartesianNed(srcRoad.endControlPoints[i]);
					}
				}
				roads.push_back(road);
			}
			break;
			case mapfeatures::FeatureBuilding:
			{
				Building building;
				const mapfeatures::Building& srcBuilding = static_cast<const mapfeatures::Building&>(*feature);
				for (int j = 0; j < srcBuilding.points.size(); ++j)
				{
					osg::Vec3f pos = converter.latLonAltToCartesianNed(srcBuilding.points[j]);
					building.points.push_back(pos);
				}
				building.height = srcBuilding.height;
				buildings.push_back(building);
			}
			break;
			case mapfeatures::FeatureWater:
			{
				Lake lake;
				const mapfeatures::Water& srcWater = static_cast<const mapfeatures::Water&>(*feature);
				for (int j = 0; j < srcWater.points.size(); ++j)
				{
					osg::Vec3f pos = converter.latLonAltToCartesianNed(srcWater.points[j]);
					lake.points.push_back(pos);
				}

				lakes.push_back(lake);
			}
			break;
			case mapfeatures::FeatureAirport:
			{
				const mapfeatures::Airport& srcAirport = static_cast<const mapfeatures::Airport&>(*feature);
				for (const mapfeatures::Airport::Runway& srcRunway : srcAirport.runways)
				{
					Runway runway;
					runway.startPoint = converter.latLonAltToCartesianNed(toLatLonAlt(srcRunway.start, srcAirport.altitude));
					runway.endPoint = converter.latLonAltToCartesianNed(toLatLonAlt(srcRunway.end, srcAirport.altitude));

					std::vector<std::string> strs;
					boost::split(strs, srcRunway.name, boost::is_any_of("\\/"));
					if (strs.size() == 2)
					{
						runway.startMarking = strs.front();
						runway.endMarking = strs.back();
					}

					runway.width = srcRunway.width;
					runways.push_back(runway);
				}
				if (0)
				{
					for (const LatLonPoints& polygon : srcAirport.areaPolygons)
					{
						PolyRegion region;
						for (int j = 0; j < polygon.size(); ++j)
						{
							region.points.push_back(converter.latLonAltToCartesianNed(toLatLonAlt(polygon[j], srcAirport.altitude)));
						}
						polyRegions.push_back(region);
					}
				}
			}
			break;
			assert(!"Not implemented");
			}
		}

		osg::ref_ptr<osg::Program> modelProgram = mPrograms->getRequiredProgram("model");

		// Create roads
		if (!roads.empty())
		{
			RoadsBatchPtr visRoads(new RoadsBatch(roads, mPrograms->getRequiredProgram("road")));
			objects.nodes[PlanetFeaturesParams::groupsNonBuildingsIndex].push_back(visRoads);
		}

		// Create poly regions
		if (!polyRegions.empty())
		{
			RoadsBatchPtr visPolyRegions(new RoadsBatch(polyRegions, modelProgram));
			objects.nodes[PlanetFeaturesParams::groupsNonBuildingsIndex].push_back(visPolyRegions);
		}

		// Create runways
		// Runways are created after poly regions to ensure runways draw on top.
		if (!runways.empty())
		{
			RunwaysBatchPtr visRunways(new RunwaysBatch(runways, modelProgram, mPrograms->getRequiredProgram("modelText")));
			objects.nodes[PlanetFeaturesParams::groupsNonBuildingsIndex].push_back(visRunways);
		}

		// Create batches for rendering
		if (!buildings.empty())
		{
			BuildingsBatchPtr visBuildings(new BuildingsBatch(buildings, mPrograms->getRequiredProgram("building"), mBuildingTypes));
			objects.nodes[PlanetFeaturesParams::groupsBuildingsIndex].push_back(visBuildings);
		}

		// Create lakes
		if (!lakes.empty())
		{
			LakesConfig visLakesConfig;
			visLakesConfig.program = mPrograms->getRequiredProgram("lake");
			visLakesConfig.waterStateSet = mWaterStateSet;

			LakesBatchPtr visLakes(new LakesBatch(lakes, visLakesConfig));
			objects.nodes[PlanetFeaturesParams::groupsNonBuildingsIndex].push_back(visLakes);
		}

		return objectsPtr;
	}

private:
	const ElevationProviderPtr mLatLonElevationProvider;
	std::shared_mutex* mElevationProviderMutex;
	const ShaderPrograms* mPrograms;
	osg::ref_ptr<osg::StateSet> mWaterStateSet;
	BuildingTypesPtr mBuildingTypes;
};

std::unique_ptr<FeatureTile> createTile(const QuadTreeTileKey& key, const LatLonBounds& bounds)
{
	VisFeatureTile* tile = new VisFeatureTile;
	tile->key = key;
	tile->bounds = bounds;
	return std::unique_ptr<FeatureTile>(tile);
};

PlanetFeatures::PlanetFeatures(const PlanetFeaturesParams& params) :
	mScheduler(params.scheduler),
	mVisObjectsLoadTask(new VisObjectsLoadTask(params.programs, params.waterStateSet, params.buildingTypes)),
	mPlanetRadius(params.planetRadius),
	mFileLocator(params.fileLocator),
	mTilesDirectoryRelAssetPackage(params.tilesDirectoryRelAssetPackage),
	mFeatures(createTile)
{
	assert(mScheduler);

	for (int i = 0; i < PlanetFeaturesParams::featureGroupsSize; ++i)
	{
		mGroups[i] = params.groups[i];
	}

	for (const auto& path : params.treeFiles)
	{
		mapfeatures::addJsonFileTilesToTree(mFeatures, path.string());
	}
}

bool shouldTraverse(const VisFeatureTile& tile) { return tile.loaded; }
bool shouldKeep(const VisFeatureTile& tile) { return false; }

PlanetFeatures::~PlanetFeatures()
{
	// Cancel all loading tasks and wait for them to finish
	for (const LoadingItemPtr& item : mLoadingQueue)
	{
		item->cancel = true;
	}
	mScheduler->waitFor(mLoadingTaskSync);

	for (LoadedVisObjects* object : mLoadedVisObjects)
	{
		unload(*object);
	}
}

void PlanetFeatures::onSurfaceTileAdded(const QuadTreeTileKey& key)
{
	auto& tree = (skybolt::DiQuadTree<VisFeatureTile>&)(mFeatures.tree);

	auto visiter = [this](VisFeatureTile& tile) { loadTile(tile); };

	QuadTreeTileKey rootKey = createAncestorKey(key, 0);
	VisFeatureTile* root;
	if (rootKey == tree.leftTree.getRoot().key)
	{
		root = &tree.leftTree.getRoot();
	}
	else if (rootKey == tree.rightTree.getRoot().key)
	{
		root = &tree.rightTree.getRoot();
	}
	else
	{
		return;
	}

	VisFeatureTile& tile = visitHierarchyToKey<VisFeatureTile>(*root, key, visiter);

	// Prune descendents
	if (tile.hasChildren())
	{
		auto pruner = [this](VisFeatureTile& tile) { unloadTile(tile); };
		for (int i = 0; i < 4; ++i)
		{
			pruneTree<VisFeatureTile>(static_cast<VisFeatureTile&>(*tile.children[i]), shouldTraverse, shouldKeep, pruner);
		}
	}
}

void PlanetFeatures::updatePreRender(const RenderContext& context)
{
	processLoadingQueue();

	auto& tree = (skybolt::DiQuadTree<VisFeatureTile>&)(mFeatures.tree);

	for (LoadedVisObjects* objects : mLoadedVisObjects)
	{
		updatePreRender(*objects, context);
	}
}

void PlanetFeatures::loadTile(VisFeatureTile& tile)
{
	if (!tile.loaded)
	{
		if (tile.featureCountInFile > 0)
		{
			tile.loaded = true;
			std::string filename = mFileLocator(mTilesDirectoryRelAssetPackage + "/" + mapfeatures::getTilePathFromKey(tile.key), file::FileLocatorMode::Required).string();
			sim::LatLon origin = tile.bounds.center();

			LoadingItemPtr loadingItem(new LoadingItem);
			loadingItem->tile = &tile;
			mLoadingQueue.push_back(loadingItem);

			mScheduler->run([=]()
			{
				if (!loadingItem->cancel) // if Tile hasn't been canceled by the time the scheduled task runs
				{
					std::vector<mapfeatures::FeaturePtr> features;
					mapfeatures::loadTile(filename, features);

					loadingItem->objects = mVisObjectsLoadTask->loadVisObjects(features, origin, mPlanetRadius);
				}
			}, &mLoadingTaskSync);
		}
	}
}

void PlanetFeatures::unloadTile(VisFeatureTile& tile)
{
	assert(tile.loaded);
	tile.loaded = false;

	LoadedVisObjects* objects = tile.visObjects.get();
	if (objects)
	{
		unload(*objects);

		auto it = std::find(mLoadedVisObjects.begin(), mLoadedVisObjects.end(), tile.visObjects.get());
		if (it != mLoadedVisObjects.end())
		{
			mLoadedVisObjects.erase(it);
		}
		tile.visObjects.reset();
	}
}

void PlanetFeatures::processLoadingQueue()
{
	static const int maxItemsPerUpdate = 1;
	int loadedItems = 0;
	for (size_t i = 0; i < mLoadingQueue.size();)
	{
		LoadingItem& item = *mLoadingQueue[i];

		bool erase = !item.tile->loaded;
		if (erase)
		{
			item.cancel = true;
		}
		else if (loadedItems < maxItemsPerUpdate)
		{
			std::unique_ptr<LoadedVisObjects>& objects = item.objects;
			if (objects)
			{
				cxxtimer::Timer timer;
				timer.start();

				for (int i = 0; i < PlanetFeaturesParams::featureGroupsSize; ++i)
				{
					for (const VisObjectPtr& node : objects->nodes[i])
					{
						osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node->_getNode());
						if (transform)
						{
							osg::Vec3d pos = llaToGeocentric(osg::Vec2d(objects->latLonOrigin.lat, objects->latLonOrigin.lon), 0, mPlanetRadius);
							osg::Matrixd mat = osg::Matrixd::translate(pos);
							mat.setRotate(latLonToGeocentricLtpOrientation(osg::Vec2d(objects->latLonOrigin.lat, objects->latLonOrigin.lon)));

							transform->setMatrix(mat);
							mGroups[i]->addChild(transform);
						}
					}
				}

				item.tile->visObjects = std::move(objects);
				mLoadedVisObjects.push_back(item.tile->visObjects.get());
				erase = true;
				++loadedItems;

				//printf("Feature %lli\n", timer.count());
				timer.reset();
			}
		}

		if (erase)
		{
			mLoadingQueue.erase(mLoadingQueue.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void PlanetFeatures::updatePreRender(LoadedVisObjects& objects, const RenderContext& context) const
{
	for (int i = 0; i < PlanetFeaturesParams::featureGroupsSize; ++i)
	{
		for (const VisObjectPtr& node : objects.nodes[i])
		{
			node->updatePreRender(context);
		}
	}
}

void PlanetFeatures::unload(LoadedVisObjects& objects) const
{
	for (int i = 0; i < PlanetFeaturesParams::featureGroupsSize; ++i)
	{
		for (const VisObjectPtr& node : objects.nodes[i])
		{
			mGroups[i]->removeChild(node->_getNode());
		}
	}
}

} // namespace vis
} // namespace skybolt
