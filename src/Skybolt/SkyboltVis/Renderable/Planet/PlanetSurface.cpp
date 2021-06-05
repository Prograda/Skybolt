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
#include "SkyboltVis/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/Renderable/Planet/Tile/AsyncTileLoader.h"
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
		mPlanetTileSources(config.planetTileSources),
		mRadius(config.radius),
		mParentTransform(config.parentTransform),
		mForestGroup(config.forestGroup),
		mGroup(new osg::Group)
	{
		assert(mPlanetTileSources.albedo);
		assert(mPlanetTileSources.elevation);

		mParentTransform->addChild(mGroup);

		osg::StateSet* ss = mGroup->getOrCreateStateSet();
		ss->setAttribute(config.programs->planet);

		if (config.oceanEnabled)
		{
			ss->setDefine("ENABLE_OCEAN");
		}

#ifdef WIREFRAME
		osg::PolygonMode * polygonMode = new osg::PolygonMode;
		polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

		ss->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif

		OsgTileFactoryConfig factoryConfig;
		factoryConfig.scheduler = config.scheduler;
		factoryConfig.programs = config.programs;
		factoryConfig.shadowMaps = config.shadowMaps;
		factoryConfig.planetRadius = mRadius;
		factoryConfig.forestGeoVisibilityRange = config.forestGeoVisibilityRange;
		factoryConfig.hasCloudShadows = config.cloudsTexture != nullptr;
		mOsgTileFactory = std::make_unique<OsgTileFactory>(factoryConfig);

		mPredicate = std::make_shared<PlanetSubdivisionPredicate>();
		mPredicate->maxLevel = std::max(config.albedoMaxLodLevel, config.elevationMaxLodLevel);
		mPredicate->observerAltitude = 20;
		mPredicate->observerLatLon = osg::Vec2(0, 0);
		mPredicate->planetRadius = mRadius;

		auto imageLoader = std::make_shared<PlanetTileImagesLoader>();
		imageLoader->elevationLayer = mPlanetTileSources.elevation;
		imageLoader->landUseLayer = mPlanetTileSources.attribute;
		imageLoader->albedoLayer = mPlanetTileSources.albedo;
		imageLoader->maxElevationLod = config.elevationMaxLodLevel;
		imageLoader->minAttributeLod = 9;

		AsyncTileLoaderPtr loader(new AsyncTileLoader(imageLoader, config.scheduler));

		mTileSource.reset(new QuadTreeTileLoader(loader, mPredicate));
	}

	PlanetSurface::~PlanetSurface()
	{
		mParentTransform->removeChild(mGroup);

		// Remove forest OSG nodes
		for (const auto& item : mTileNodes)
		{
			const OsgTile& tile = item.second;
			if (tile.forest)
			{
				mForestGroup->removeChild(tile.forest->_getNode());
			}
		}
	}

	typedef std::map<QuadTreeTileKey, const AsyncQuadTreeTile*> QuadTreeTilesMap;

	void PlanetSurface::updateGeometry()
	{
		std::vector<AsyncQuadTreeTile*> addedTiles;
		std::vector<QuadTreeTileKey> removedTiles;

		mTileSource->update(addedTiles, removedTiles);

		for (const QuadTreeTileKey& key : removedTiles)
		{
			auto it = mTileNodes.find(key);
			if (it != mTileNodes.end())
			{
				const OsgTile& tile = it->second;
				mGroup->removeChild(tile.transform);
				if (tile.forest)
				{
					mForestGroup->removeChild(tile.forest->_getNode());
				}
				mTileNodes.erase(it);
			}
			CALL_LISTENERS(tileRemovedFromSceneGraph(key), Listenable<PlanetSurfaceListener>::mListeners);
		}

		for (AsyncQuadTreeTile* tile : addedTiles)
		{
			assert(tile && tile->getData());

			const PlanetTileImages& images = static_cast<const PlanetTileImages&>(*tile->getData());
			Box2d latLonBounds(math::vec2SwapComponents(tile->bounds.minimum), math::vec2SwapComponents(tile->bounds.maximum));
			OsgTile osgTile = mOsgTileFactory->createOsgTile(tile->key, latLonBounds, images.heightMapImage, images.landMaskImage, images.albedoMapImage, images.attributeMapImage);

			mGroup->addChild(osgTile.transform);
			mTileNodes[tile->key] = osgTile;

			if (osgTile.forest)
			{
				mForestGroup->addChild(osgTile.forest->_getNode());
			}
			CALL_LISTENERS(tileAddedToSceneGraph(tile->key), Listenable<PlanetSurfaceListener>::mListeners);
		}
	}

	sim::LatLon toLatLon(const osg::Vec2d& latLon)
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

			if (tile.forest)
			{
				osg::Vec3d tilePos = llaToGeocentric(tile.tileCenter, 0, mRadius) * mParentTransform->getMatrix();
				tilePos.z() = mParentTransform->getMatrix().getTrans().z() - mRadius;

				tile.forest->setPosition(tilePos);
				tile.forest->update(mPredicate->observerLatLon - tile.tileCenter); // TODO: this is the latLon camera position. Make this clearer.
				tile.forest->updatePreRender(context); // TODO: the Scene should be calling this, not the PlanetSurface
			}
		}
	}

} // namespace vis
} // namespace skybolt
