/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgTileFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileKeyHelpers.h"
#include "SkyboltVis/Elevation/HeightMapElevationProvider.h"
#include "SkyboltVisOsg/OsgMathHelpers.h"
#include "SkyboltVisOsg/OsgGeocentric.h"
#include "SkyboltVisOsg/OsgImageHelpers.h"
#include "SkyboltVisOsg/OsgStateSetHelpers.h"
#include "SkyboltVisOsg/OsgTextureHelpers.h"
#include "SkyboltVisOsg/Renderable/Planet/PlanetTileGeometry.h"
#include "SkyboltVisOsg/Shader/ShaderProgramRegistry.h"

#include <osg/Geode>

using namespace skybolt;

namespace skybolt {
namespace vis {

// Quad UVs follow OpenGL standard where 0,0 is at bottom left of image,
// but we want origin to be at top left so that tiles appear right-side-up.
constexpr bool osgTileFactoryFlipV = true;

OsgTileFactory::OsgTileFactory(const OsgTileFactoryConfig& config) :
	mPrograms(config.programs),
	mDetailMappingTechnique(config.detailMappingTechnique),
	mPlanetRadius(config.planetRadius),
	mHasCloudShadows(config.hasCloudShadows),
	mHeightMapTexelsOnTileEdge(config.heightMapTexelsOnTileEdge)
{
	assert(mPrograms);
}

OsgTileFactory::~OsgTileFactory() = default;

static osg::Vec2f toOsgVec2f(const glm::vec2& v)
{
	return osg::Vec2f(v.x, v.y);
}

OsgTile OsgTileFactory::createOsgTile(const QuadTreeTileKey& key, const Box2d& latLonBounds, const TileTextures& textures) const
{
	// Get heightmap for tile, and its scale and offset relative to the tile
	glm::vec2 heightImageScale, heightImageOffset;
	getTileTransformInParentSpace(key, textures.height.key.level, heightImageScale, heightImageOffset, osgTileFactoryFlipV);

	if (mHeightMapTexelsOnTileEdge)
	{
		const auto& image = textures.height.texture->getImage();
		ScaleOffset scaleOffset = calcHalfTexelEdgeRemovalScaleOffset(osg::Vec2i(image->s(), image->t()));
		heightImageScale = math::componentWiseMultiply(heightImageScale, scaleOffset.scale);
		heightImageOffset = math::componentWiseMultiply(heightImageOffset, scaleOffset.scale) + scaleOffset.offset;
	}


	OsgTile result;

	// Create terrain
	glm::dvec2 centerLatLon = latLonBounds.center();
	osg::Vec3d tilePosition = llaToGeocentric(osg::Vec2d(centerLatLon.x, centerLatLon.y), 0, mPlanetRadius);

	result.transform = new osg::MatrixTransform;
	osg::Matrix matrix;
	matrix.setTrans(tilePosition);
	result.transform->setMatrix(matrix);

	result.modelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	result.transform->getOrCreateStateSet()->addUniform(result.modelMatrixUniform);

	glm::vec2 albedoImageScale, albedoImageOffset;
	getTileTransformInParentSpace(key, textures.albedo.key.level, albedoImageScale, albedoImageOffset, osgTileFactoryFlipV);

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

		glm::vec2 attributeImageScale, attributeImageOffset;
		if (textures.attribute)
		{
			getTileTransformInParentSpace(key, textures.attribute->key.level, attributeImageScale, attributeImageOffset, osgTileFactoryFlipV);
		}

		TerrainConfig config;
		config.program = mPrograms->getRequiredProgram("terrainPlanetTile");
		config.tile = planetTile;
		config.heightMap = textures.height.texture;
		config.normalMap = textures.normal;
		config.landMask = textures.landMask;
		config.overallAlbedoMap = textures.albedo.texture;
		config.attributeMap = textures.attribute ? textures.attribute->texture : nullptr;
		config.elevationRerange = textures.elevationRerange;

		config.heightMapUvScale = toOsgVec2f(heightImageScale);
		config.heightMapUvOffset = toOsgVec2f(heightImageOffset);
		config.overallAlbedoMapUvScale = toOsgVec2f(albedoImageScale);
		config.overallAlbedoMapUvOffset = toOsgVec2f(albedoImageOffset);
		config.attributeMapUvScale = toOsgVec2f(attributeImageScale);
		config.attributeMapUvOffset = toOsgVec2f(attributeImageOffset);
		config.detailMappingTechnique = mDetailMappingTechnique;

		result.highResTerrain.reset(new Terrain(config));
		result.transform->addChild(result.highResTerrain->getTerrainNode());
	}
	else if (textures.albedo.texture)
	{
		// Low LOD terrain
		osg::ref_ptr<osg::Geode> geode = createPlanetTileGeode(tilePosition, latLonBounds, mPlanetRadius, Triangles);
		result.transform->addChild(geode);

		int unit = 0;
		osg::StateSet* ss = geode->getOrCreateStateSet();
		ss->addUniform(new osg::Uniform("heightMapUvScale", toOsgVec2f(heightImageScale)));
		ss->addUniform(new osg::Uniform("heightMapUvOffset", toOsgVec2f(heightImageOffset)));

		ss->addUniform(new osg::Uniform("albedoImageScale", toOsgVec2f(albedoImageScale)));
		ss->addUniform(new osg::Uniform("albedoImageOffset", toOsgVec2f(albedoImageOffset)));

		ss->setTextureAttributeAndModes(unit, textures.albedo.texture);
		ss->addUniform(createUniformSampler2d("albedoSampler", unit++));


		ss->setTextureAttributeAndModes(unit, textures.landMask);
		ss->addUniform(createUniformSampler2d("landMaskSampler", unit++));
		// TODO: set clamping mode. Height points should be on edges of terrain
	}

	if (mHasCloudShadows)
	{
		glm::vec2 cloudImageScale, cloudImageOffset;
		getTileTransformInParentSpace(key, 0, cloudImageScale, cloudImageOffset, osgTileFactoryFlipV);

		// Transform the cloud texture coordinates to account for there being only one cloud texture stretched across two tiles in X
		cloudImageScale.x *= 0.5;
		cloudImageOffset.x *= 0.5;
		if (key.x >= 1 << key.level)
		{
			cloudImageOffset.x += 0.5;
		}

		osg::StateSet* ss = result.transform->getOrCreateStateSet();

		ss->addUniform(new osg::Uniform("cloudUvScaleRelTerrainUv", toOsgVec2f(cloudImageScale)));
		ss->addUniform(new osg::Uniform("cloudUvOffsetRelTerrainUv", toOsgVec2f(cloudImageOffset)));
	}

	return result;
}

} // namespace vis
} // namespace skybolt
