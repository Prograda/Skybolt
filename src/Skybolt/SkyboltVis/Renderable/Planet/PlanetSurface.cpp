/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetSurface.h"
#include "PlanetTileGeometry.h"
#include "TextureCompiler.h"
#include "SkyboltVis/LlaToNedConverter.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Renderable/Forest/GpuForestTile.h"
#include "SkyboltVis/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/Renderable/Planet/Tile/ConcurrentAsyncTileLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/OsgTile.h"
#include "SkyboltVis/Renderable/Planet/Tile/OsgTileFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/QuadTreeTileLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetSubdivisionPredicate.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetTileImagesLoader.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <cxxtimer/cxxtimer.hpp>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace skybolt;

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

namespace skybolt {
namespace vis {

PlanetSurface::PlanetSurface(const PlanetSurfaceConfig& config) :
	mRadius(config.radius),
	mParentTransform(config.parentTransform),
	mOsgTileFactory(config.osgTileFactory),
	mTileTexturesProvider(config.tileTexturesProvider),
	mGpuForest(config.gpuForest),
	mGroup(new osg::Group)
{
	auto planetTileSources = config.planetTileSources;
	assert(planetTileSources.albedo);
	assert(planetTileSources.elevation);

	mParentTransform->addChild(mGroup);

	osg::StateSet* ss = mGroup->getOrCreateStateSet();
	ss->setAttribute(config.programs->getRequiredProgram("planet"));

	if (config.oceanEnabled)
	{
		ss->setDefine("ENABLE_OCEAN");
	}

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

	ss->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif
	mPredicate = std::make_shared<PlanetSubdivisionPredicate>();
	mPredicate->maxLevel = std::max(std::max(config.albedoMaxLodLevel, config.elevationMaxLodLevel), config.attributeMaxLodLevel);
	mPredicate->observerAltitude = 20;
	mPredicate->observerLatLon = osg::Vec2(0, 0);
	mPredicate->planetRadius = mRadius;

	auto imageLoader = std::make_shared<PlanetTileImagesLoader>(config.radius);
	imageLoader->elevationLayer = planetTileSources.elevation;
	imageLoader->attributeLayer = planetTileSources.attribute;
	imageLoader->albedoLayer = planetTileSources.albedo;
	imageLoader->maxElevationLod = config.elevationMaxLodLevel;
	imageLoader->minAttributeLod = config.attributeMinLodLevel;
	imageLoader->maxAttributeLod = config.attributeMaxLodLevel;

	AsyncTileLoaderPtr loader(new ConcurrentAsyncTileLoader(imageLoader, config.scheduler));

	mTileSource.reset(new QuadTreeTileLoader(loader, mPredicate));
}

PlanetSurface::~PlanetSurface()
{
	mParentTransform->removeChild(mGroup);
}

void PlanetSurface::updateGeometry()
{
	mTileSource->update();

	// Get added and removed tile images
	TileKeyImagesMap addedTiles;
	std::set<QuadTreeTileKey> removedTiles;
	QuadTreeTileLoader::LoadedTileTreePtr tree = mTileSource->getLoadedTree();

	{
		TileKeyImagesMap currentLeafTileImages;
		findLeafTiles(*tree, currentLeafTileImages);
		findAddedAndRemovedTiles(mLeafTileImages, currentLeafTileImages, addedTiles, removedTiles);
		std::swap(mLeafTileImages, currentLeafTileImages);
	}

	// Remove OSG nodes for removed tiles
	for (const QuadTreeTileKey& key : removedTiles)
	{
		auto it = mTileNodes.find(key);
		if (it != mTileNodes.end())
		{
			const OsgTile& tile = it->second;
			mGroup->removeChild(tile.transform);
			mTileNodes.erase(it);
		}
		CALL_LISTENERS(tileRemovedFromSceneGraph(key));
	}

	// Create OSG nodes for added tiles
	for (const auto& [key, tile] : addedTiles)
	{
		assert(tile);
		const PlanetTileImages& images = static_cast<const PlanetTileImages&>(*tile);

		auto textureTiles = mTileTexturesProvider(images);
		auto bounds = getKeyLonLatBounds<osg::Vec2d>(key);
		Box2d latLonBounds(math::vec2SwapComponents(bounds.minimum), math::vec2SwapComponents(bounds.maximum));
		OsgTile osgTile = mOsgTileFactory->createOsgTile(key, latLonBounds, textureTiles);

		mGroup->addChild(osgTile.transform);
		mTileNodes[key] = osgTile;

		CALL_LISTENERS(tileAddedToSceneGraph(key));
	}

	if (mGpuForest)
	{
		mGpuForest->updateFromTree(*tree);
	}
}

static sim::LatLon toLatLon(const osg::Vec2d& latLon)
{
	return sim::LatLon(latLon.x(), latLon.y());
}

void PlanetSurface::updatePreRender(const RenderContext& context)
{
	osg::Vec3d geocentricPos = context.camera.getPosition() * osg::Matrix::inverse(mParentTransform->getMatrix());

#ifdef DEBUG_TILE_STRUCTURE_CAMERA_BEHAVIOR
	{
		// Don't move tile-structure origin camera unless it has moved a very long way
		static osg::Vec3d fixedPos = geocentricPos;

		if ((fixedPos - geocentricPos).length() < 100000)
			geocentricPos = fixedPos;
		else
			fixedPos = geocentricPos;
	}
#endif

	geocentricToLla(geocentricPos, mPredicate->observerLatLon, mPredicate->observerAltitude, mRadius);

	updateGeometry();

	LlaToNedConverter converter(toLatLon(mPredicate->observerLatLon), boost::none);
	for (const auto& node : mTileNodes)
	{
		const OsgTile& tile = node.second;
		osg::Matrix modelMatrix = tile.transform->getWorldMatrices().front();
		tile.modelMatrixUniform->set(modelMatrix);

		if (tile.highResTerrain)
		{
			tile.highResTerrain->updatePreRender(context);
		}
	}

	if (mGpuForest)
	{
		mGpuForest->updatePreRender(context);
	}
}

} // namespace vis
} // namespace skybolt
