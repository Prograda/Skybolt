/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgTileFactory.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/ShaderProgramRegistry.h"
#include "SkyboltVis/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/ElevationProvider/HeightmapElevationProvider.h"
#include "SkyboltVis/Renderable/Planet/PlanetTileGeometry.h"
#include "SkyboltVis/Renderable/Planet/TextureCompiler.h"

#include <osg/Geode>

using namespace skybolt;

namespace skybolt {
namespace vis {

void getTileTransformInParentSpace(const QuadTreeTileKey& key, int parentLod, osg::Vec2f& scale, osg::Vec2f& offset)
{
	int reductions = key.level - parentLod;
	assert(reductions >= 0);

	int scaleInt = 1 << reductions;

	osg::Vec2i reducedIndex(key.x / scaleInt, key.y / scaleInt);
	osg::Vec2i v = reducedIndex * scaleInt;

	float rcpScale = 1.0f / (float)scaleInt;
	scale = osg::Vec2f(rcpScale, rcpScale);
	offset = osg::Vec2f(key.x - v.x(), (scaleInt - 1) - (key.y - v.y())) * rcpScale;
}

OsgTileFactory::OsgTileFactory(const OsgTileFactoryConfig& config) :
	mScheduler(config.scheduler),
	mPrograms(config.programs),
	mShadowMaps(config.shadowMaps),
	mPlanetRadius(config.planetRadius),
	mForestGeoVisibilityRange(config.forestGeoVisibilityRange),
	mHasCloudShadows(config.hasCloudShadows)
{
	assert(mScheduler);
	assert(mPrograms);}

OsgTile OsgTileFactory::createOsgTile(const QuadTreeTileKey& key, const Box2d& latLonBounds, const TileImage& heightImage, osg::Image* landMaskImage,
	const TileImage& albedoImage, const TileImage& attributeImage) const
{
	// Get heightmap for tile, and its scale and offset relative to the tile
	osg::Vec2f heightImageScale, heightImageOffset;
	getTileTransformInParentSpace(key, heightImage.key.level, heightImageScale, heightImageOffset);

	// Modify scale and offset to remove heightmap border
	osg::Image* heightImagePr = heightImage.image;
	osg::Vec2f borderRemovalScale(float(heightImagePr->s() - 2) / (float)heightImagePr->s(), float(heightImagePr->t() - 2) / (float)heightImagePr->t());
	osg::Vec2f borderRemovalOffset(1.0f / (float)heightImagePr->s(), 1.0f / (float)heightImagePr->t());
	heightImageScale = math::componentWiseMultiply(heightImageScale, borderRemovalScale);
	heightImageOffset = math::componentWiseMultiply(heightImageOffset, borderRemovalScale) + borderRemovalOffset;

	OsgTile result;

	// Find heightmap bounds
	osg::Vec2d offsetNe(heightImageOffset.y(), heightImageOffset.x());
	osg::Vec2d scaleNe = math::vec2SwapComponents(heightImageScale);

	result.heightImage = heightImage.image;
	result.heightImageLatLonBounds.minimum = math::componentWiseLerp(latLonBounds.minimum, latLonBounds.maximum, math::componentWiseDivide(-offsetNe, scaleNe));
	result.heightImageLatLonBounds.maximum = math::componentWiseLerp(latLonBounds.minimum, latLonBounds.maximum, math::componentWiseDivide(osg::Vec2d(1, 1) - offsetNe, scaleNe));


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
	getTileTransformInParentSpace(key, albedoImage.key.level, albedoImageScale, albedoImageOffset);

	if (key.level >= 6) // TODO: automate this value
	{
		// High LOD terrain
		std::shared_ptr<TerrainConfig::PlanetTile> planetTile(new TerrainConfig::PlanetTile);
		planetTile->latLonBounds = latLonBounds;
		planetTile->planetRadius = mPlanetRadius;

		osg::Vec2f heightImageLatLonDelta = math::componentWiseDivide(osg::Vec2f(latLonBounds.size()), heightImageScale);
		osg::Vec2f texelWorldSize = osg::Vec2f(heightImageLatLonDelta.x() * mPlanetRadius / heightImagePr->t(), heightImageLatLonDelta.y() * mPlanetRadius * cos(latLonBounds.center().x()) / heightImagePr->s()); // note flip: s,t -> y,x

		osg::Vec2f attributeImageScale, attributeImageOffset;
		getTileTransformInParentSpace(key, attributeImage.key.level, attributeImageScale, attributeImageOffset);

		TerrainConfig config = mCommonTerrainConfig;
		config.program = mPrograms->terrainPlanetTile;
		config.tile = planetTile;

		if (mCacheHeight.putOrGet(heightImage.image, config.heightMap))
		{
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(heightImage.image);
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			config.heightMap = texture;
		};

		if (mCacheNormal.putOrGet(heightImage.image, config.normalMap))
		{
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(createNormalmapFromHeightmap(*heightImage.image, texelWorldSize));
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			// Disable mip-mapping because it's unnecessary on tiles and expensive to generate the mipmaps,
			// and we need tile textures to load as quickly as possible.
			// We ensure the correctly sized tile is loaded for tile's screenspace size,
			// which makes mip-mapping unnecessary.
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			config.normalMap = texture;
		};

		if (mCacheAlbedo.putOrGet(albedoImage.image, config.overallAlbedoMap))
		{
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(albedoImage.image);
			texture->setInternalFormat(toSrgbInternalFormat(texture->getInternalFormat()));
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			config.overallAlbedoMap = texture;
		};

		if (config.detailMaps && attributeImage.image)
		{
			if (mCacheAttribute.putOrGet(attributeImage.image, config.detailMaps->attributeMap))
			{
				osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(attributeImage.image);
				texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
				texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
				texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
				texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
				config.detailMaps->attributeMap = texture;
			}
		}
		else
		{
			config.detailMaps.reset();
		}
		
		config.heightMapUvScale = heightImageScale;
		config.heightMapUvOffset = heightImageOffset;
		config.overallAlbedoMapUvScale = albedoImageScale;
		config.overallAlbedoMapUvOffset = albedoImageOffset;
		config.shadowMaps = mShadowMaps;

		result.highResTerrain.reset(new Terrain(config));
		result.transform->addChild(result.highResTerrain->getTerrainNode());
	}
	else if (albedoImage.image)
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

		osg::ref_ptr<osg::Texture2D> texture;
		if (mCacheAlbedo.putOrGet(albedoImage.image, texture))
		{
			texture = new osg::Texture2D(albedoImage.image);
			texture->setInternalFormat(toSrgbInternalFormat(texture->getInternalFormat()));
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			// TODO: set clamping mode. Height points should be on edges of terrain
		}
		ss->setTextureAttributeAndModes(unit, texture);
		ss->addUniform(createUniformSampler2d("albedoSampler", unit++));

		// TODO: fix land mask not lining up exactly with albedo map
		if (mCacheLandMask.putOrGet(landMaskImage, texture))
		{
			texture = new osg::Texture2D(landMaskImage);
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		}
		ss->setTextureAttributeAndModes(unit, texture);
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

	// Create forest
	if (attributeImage.image && attributeImage.key.level > 11)
	{
		const Box2d attributeImageLatLonBounds = getKeyLatLonBounds<osg::Vec2d>(attributeImage.key);

		Box2f boundsRelLatLon;
		boundsRelLatLon.minimum = attributeImageLatLonBounds.minimum - centerLatLon;
		boundsRelLatLon.maximum = attributeImageLatLonBounds.maximum - centerLatLon;

		Box2f heightImageBoundsRelLatLon;
		heightImageBoundsRelLatLon.minimum = result.heightImageLatLonBounds.minimum - centerLatLon;
		heightImageBoundsRelLatLon.maximum = result.heightImageLatLonBounds.maximum - centerLatLon;

		double cosLat = cos(centerLatLon.x());

		float visibilityRangeRadiansLon = mForestGeoVisibilityRange / mPlanetRadius;
		osg::Vec2f pageSize(visibilityRangeRadiansLon, visibilityRangeRadiansLon / cosLat);
		ElevationProviderPtr elevationProvider(new HeightmapElevationProvider(heightImage.image, heightImageBoundsRelLatLon));

		osg::Matrix matrix;
		matrix.setTrans(tilePosition);
		osg::ref_ptr<osg::MatrixTransform> transform(new osg::MatrixTransform(matrix));

		Vec2Transform converter = [=](const osg::Vec2f& p)
		{
			return osg::Vec2f(p.x() * mPlanetRadius, p.y() * mPlanetRadius * cosLat);
		};

		result.forest.reset(new PagedForest(*mScheduler, attributeImage.image, mPrograms, elevationProvider, boundsRelLatLon, pageSize, converter, mForestGeoVisibilityRange));
		result.tileCenter = centerLatLon;
	}

	return result;
}

} // namespace vis
} // namespace skybolt
