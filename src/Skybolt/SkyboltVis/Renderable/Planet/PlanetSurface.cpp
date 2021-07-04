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
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Renderable/Forest/GpuForestTile.h"
#include "SkyboltVis/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/Renderable/Planet/Tile/AsyncTileLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/OsgTile.h"
#include "SkyboltVis/Renderable/Planet/Tile/OsgTileFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/QuadTreeTileLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetSubdivisionPredicate.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetTileImagesLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileTextureCache.h"
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

static osg::ref_ptr<osg::Texture2D> createNonSrgbTextureWithoutMipmaps(const osg::ref_ptr<osg::Image>& image)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	return texture;
}

static osg::ref_ptr<osg::Texture2D> createSrgbTexture(const osg::ref_ptr<osg::Image>& image)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
	texture->setInternalFormat(toSrgbInternalFormat(texture->getInternalFormat()));
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	return texture;
}

PlanetSurface::PlanetSurface(const PlanetSurfaceConfig& config) :
	mPlanetTileSources(config.planetTileSources),
	mRadius(config.radius),
	mParentTransform(config.parentTransform),
	mOsgTileFactory(config.osgTileFactory),
	mGpuForest(config.gpuForest),
	mGroup(new osg::Group),
	mTextureCache(std::make_unique<TileTextureCache>())
{
	assert(mPlanetTileSources.albedo);
	assert(mPlanetTileSources.elevation);

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
	imageLoader->elevationLayer = mPlanetTileSources.elevation;
	imageLoader->attributeLayer = mPlanetTileSources.attribute;
	imageLoader->albedoLayer = mPlanetTileSources.albedo;
	imageLoader->maxElevationLod = config.elevationMaxLodLevel;
	imageLoader->minAttributeLod = config.attributeMinLodLevel;
	imageLoader->maxAttributeLod = config.attributeMaxLodLevel;

	AsyncTileLoaderPtr loader(new AsyncTileLoader(imageLoader, config.scheduler));

	mTileSource.reset(new QuadTreeTileLoader(loader, mPredicate));
}

PlanetSurface::~PlanetSurface()
{
	mParentTransform->removeChild(mGroup);
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
			mTileNodes.erase(it);
		}
		CALL_LISTENERS(tileRemovedFromSceneGraph(key));
	}

	std::vector<GpuForest::TileTextures> addedAttributeTiles;

	for (AsyncQuadTreeTile* tile : addedTiles)
	{
		assert(tile && tile->getData());

		const PlanetTileImages& images = static_cast<const PlanetTileImages&>(*tile->getData());
		auto textureTiles = createTileTextures(images);
		if (textureTiles.attribute)
		{
			GpuForest::TileTextures textures;
			textures.height = textureTiles.height;
			textures.attribute = *textureTiles.attribute;
			textures.key = tile->key;
			addedAttributeTiles.push_back(textures);
		}

		Box2d latLonBounds(math::vec2SwapComponents(tile->bounds.minimum), math::vec2SwapComponents(tile->bounds.maximum));
		OsgTile osgTile = mOsgTileFactory->createOsgTile(tile->key, latLonBounds, textureTiles);

		mGroup->addChild(osgTile.transform);
		mTileNodes[tile->key] = osgTile;

		CALL_LISTENERS(tileAddedToSceneGraph(tile->key));
	}

	if (mGpuForest)
	{
		mGpuForest->terrainTilesUpdated(addedAttributeTiles, removedTiles);
	}
}

OsgTileFactory::TileTextures PlanetSurface::createTileTextures(const PlanetTileImages& images)
{
	// We create some of these textures with mip-mapping disabled when it's not needed.
	// Generating mipmaps is expensive and we need tile textures to load as quickly as possible.

	OsgTileFactory::TileTextures textures;
	textures.height.texture = mTextureCache->getOrCreateTexture(TileTextureCache::TextureType::Height, images.heightMapImage.image, createNonSrgbTextureWithoutMipmaps);
	textures.height.key = images.heightMapImage.key;
	textures.normal = mTextureCache->getOrCreateTexture(TileTextureCache::TextureType::Normal, images.normalMapImage, createNonSrgbTextureWithoutMipmaps);
	textures.landMask = mTextureCache->getOrCreateTexture(TileTextureCache::TextureType::LandMask, images.landMaskImage, createNonSrgbTextureWithoutMipmaps);
	textures.albedo.texture = mTextureCache->getOrCreateTexture(TileTextureCache::TextureType::Albedo, images.albedoMapImage.image, createSrgbTexture);
	textures.albedo.key = images.albedoMapImage.key;

	if (images.attributeMapImage)
	{
		TileTexture attribute;
		attribute.texture = mTextureCache->getOrCreateTexture(TileTextureCache::TextureType::Attribute, images.attributeMapImage->image, createNonSrgbTextureWithoutMipmaps);
		attribute.key = images.attributeMapImage->key;
		textures.attribute = attribute;
	}
	return textures;
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
