/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgTileFactory.h"
#include "TileKeyHelpers.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/ElevationProvider/HeightmapElevationProvider.h"
#include "SkyboltVis/Renderable/Planet/PlanetTileGeometry.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"

#include <osg/Geode>

using namespace skybolt;

namespace skybolt {
namespace vis {

OsgTileFactory::OsgTileFactory(const OsgTileFactoryConfig& config) :
	mPrograms(config.programs),
	mDetailMappingTechnique(config.detailMappingTechnique),
	mPlanetRadius(config.planetRadius),
	mHasCloudShadows(config.hasCloudShadows)
{
	assert(mPrograms);
}

OsgTileFactory::~OsgTileFactory() = default;

OsgTile OsgTileFactory::createOsgTile(const QuadTreeTileKey& key, const Box2d& latLonBounds, const TileTextures& textures) const
{
	// Get heightmap for tile, and its scale and offset relative to the tile
	osg::Vec2f heightImageScale, heightImageOffset;
	getTileTransformInParentSpace(key, textures.height.key.level, heightImageScale, heightImageOffset);

	OsgTile result;

	// Create terrain
	osg::Vec2d centerLatLon = latLonBounds.center();
	osg::Vec3d tilePosition = llaToGeocentric(centerLatLon, 0, mPlanetRadius);

	result.transform = new osg::MatrixTransform;
	osg::Matrix matrix;
	matrix.setTrans(tilePosition);
	result.transform->setMatrix(matrix);

	result.modelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	result.transform->getOrCreateStateSet()->addUniform(result.modelMatrixUniform);

	osg::Vec2f albedoImageScale, albedoImageOffset;
	getTileTransformInParentSpace(key, textures.albedo.key.level, albedoImageScale, albedoImageOffset);

	// highLodTransitionLevel is calculated with this ad-hoc rule which works well for most planets.
	// TODO: since the main visual difference between low and high res terrain is that low res has no displacement,
	// we should really calculate the transition based on the visible height range of the terrain.
	int highLodTransitionLevel = std::max(0, int(glm::log2(float(mPlanetRadius)/500000.f)));

	if (key.level >= highLodTransitionLevel)
	{
		// High LOD terrain
		std::shared_ptr<TerrainConfig::PlanetTile> planetTile(new TerrainConfig::PlanetTile);
		planetTile->latLonBounds = latLonBounds;
		planetTile->planetRadius = mPlanetRadius;

		osg::Vec2f attributeImageScale, attributeImageOffset;
		if (textures.attribute)
		{
			getTileTransformInParentSpace(key, textures.attribute->key.level, attributeImageScale, attributeImageOffset);
		}

		TerrainConfig config;
		config.program = mPrograms->getRequiredProgram("terrainPlanetTile");
		config.tile = planetTile;
		config.heightMap = textures.height.texture;
		config.normalMap = textures.normal;
		config.landMask = textures.landMask;
		config.overallAlbedoMap = textures.albedo.texture;
		config.attributeMap = textures.attribute ? textures.attribute->texture : nullptr;

		config.heightMapUvScale = heightImageScale;
		config.heightMapUvOffset = heightImageOffset;
		config.overallAlbedoMapUvScale = albedoImageScale;
		config.overallAlbedoMapUvOffset = albedoImageOffset;
		config.attributeMapUvScale = attributeImageScale;
		config.attributeMapUvOffset = attributeImageOffset;
		config.detailMappingTechnique = mDetailMappingTechnique;

		result.highResTerrain.reset(new Terrain(config));
		result.transform->addChild(result.highResTerrain->getTerrainNode());
	}
	else if (textures.albedo.texture)
	{
		// Low LOD terrain
		osg::Geode* geode = createPlanetTileGeode(tilePosition, latLonBounds, mPlanetRadius, Triangles);
		result.transform->addChild(geode);

		int unit = 0;
		osg::StateSet* ss = geode->getOrCreateStateSet();
		ss->addUniform(new osg::Uniform("heightScale", heightImageScale));
		ss->addUniform(new osg::Uniform("heightOffset", heightImageOffset));

		ss->addUniform(new osg::Uniform("albedoImageScale", albedoImageScale));
		ss->addUniform(new osg::Uniform("albedoImageOffset", albedoImageOffset));

		ss->setTextureAttributeAndModes(unit, textures.albedo.texture);
		ss->addUniform(createUniformSampler2d("albedoSampler", unit++));


		ss->setTextureAttributeAndModes(unit, textures.landMask);
		ss->addUniform(createUniformSampler2d("landMaskSampler", unit++));
		// TODO: set clamping mode. Height points should be on edges of terrain
	}

	if (mHasCloudShadows)
	{
		osg::Vec2f cloudImageScale, cloudImageOffset;
		getTileTransformInParentSpace(key, 0, cloudImageScale, cloudImageOffset);

		// Transform the cloud texture coordinates to account for there being only one cloud texture stretched across two tiles in X
		cloudImageScale.x() *= 0.5;
		cloudImageOffset.x() *= 0.5;
		if (key.x >= 1 << key.level)
		{
			cloudImageOffset.x() += 0.5;
		}

		osg::StateSet* ss = result.transform->getOrCreateStateSet();

		ss->addUniform(new osg::Uniform("cloudUvScaleRelTerrainUv", cloudImageScale));
		ss->addUniform(new osg::Uniform("cloudUvOffsetRelTerrainUv", cloudImageOffset));
	}

	return result;
}

} // namespace vis
} // namespace skybolt
