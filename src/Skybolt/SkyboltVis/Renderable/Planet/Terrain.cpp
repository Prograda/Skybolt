/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Terrain.h"
#include "OsgGeocentric.h"
#include "OsgGeometryFactory.h"
#include "OsgGeometryHelpers.h"
#include "OsgStateSetHelpers.h"
#include "PlanetTileGeometry.h"
#include "SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h"

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osgDB/FileUtils>
#include <osg/Texture2D>
#include <osg/PatchParameter>
#include <osgDB/ReadFile>
#include <boost/lexical_cast.hpp>

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

namespace skybolt {
namespace vis {

static void createPlaneBuffersWithUvs(osg::Vec3Array& posBuffer, osg::Vec2Array& uvBuffer, osg::UIntArray& indexBuffer, const osg::Vec2f& size, int segmentCountX, int segmentCountY)
{
	osg::Vec2f halfSize = size / 2.0f;

	// vertices
	int vertexRowCount = segmentCountX + 1;
	int vertexColumnCount = segmentCountY + 1;
	int vertexCount = vertexRowCount * vertexColumnCount;
	posBuffer.reserve(vertexCount);
	uvBuffer.reserve(vertexCount);

	for (int y = 0; y < vertexColumnCount; ++y)
	{
		float sy = (float)y / (float)segmentCountY;
		for (int x = 0; x < vertexRowCount; ++x)
		{
			float sx = (float)x / (float)segmentCountX;

			osg::Vec3f position(-halfSize.x() + sx * size.x(), -halfSize.y() + sy * size.y(), 0.0f);
			posBuffer.push_back(position);

			osg::Vec2f uv(sy, sx);
			uvBuffer.push_back(uv);
		}
	}

	// indices
	int indexCount = segmentCountX * segmentCountY * 4;
	indexBuffer.reserve(indexCount);

	int i = 0;
	for (int y = 0; y < segmentCountY; ++y)
	{
		for (int x = 0; x < segmentCountX; ++x)
		{
			indexBuffer.push_back(i);
			indexBuffer.push_back(i + 1);
			indexBuffer.push_back(i + 1 + vertexRowCount);
			indexBuffer.push_back(i + vertexRowCount);

			++i;
		}
		++i;
	}
}

static osg::Node* createFlatTileGeode(const osg::Vec2f &size)
{
	int segmentCountX = 256;
	int segmentCountY = 256;
	
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::Vec2Array* uvBuffer = new osg::Vec2Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();

	createPlaneBuffersWithUvs(*posBuffer, *uvBuffer, *indexBuffer, size, segmentCountX, segmentCountY);

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    geometry->setVertexArray(posBuffer);
	geometry->setTexCoordArray(0, uvBuffer); 
	configureDrawable(*geometry);

    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::PATCHES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
    geode->addDrawable(geometry);
    return geode;
}

static void setupTerrainStateSet(osg::StateSet& ss, const TerrainConfig& config)
{
	int unit = 0;
	{
		ss.setTextureAttributeAndModes(unit, config.heightMap);
		ss.addUniform(createUniformSampler2d("heightSampler", unit++));
		// TODO: set clamping mode. Height points should be on edges of terrain
	}
	{
		ss.setTextureAttributeAndModes(unit, config.normalMap);
		ss.addUniform(createUniformSampler2d("normalSampler", unit++));
	}
	{
		ss.setTextureAttributeAndModes(unit, config.landMask);
		ss.addUniform(createUniformSampler2d("landMaskSampler", unit++));
	}

	ss.setTextureAttributeAndModes(unit, config.overallAlbedoMap);
	ss.addUniform(createUniformSampler2d("overallAlbedoSampler", unit++));

	if (config.attributeMap)
	{
		ss.setTextureAttributeAndModes(unit, config.attributeMap);
		ss.addUniform(createUniformSampler2d("attributeSampler", unit++));
	}

	if (auto uniformTechnique = dynamic_cast<const UniformDetailMappingTechnique*>(config.detailMappingTechnique.get()); uniformTechnique)
	{
		ss.setDefine("DETAIL_MAPPING_TECHNIQUE_UNIFORM");
		ss.setDefine("DETAIL_SAMPLER_COUNT");
		
		ss.setTextureAttributeAndModes(unit, uniformTechnique->albedoDetailMap);
		ss.addUniform(createUniformSampler2d("albedoDetailSamplers", unit++));
	}
	else if (auto attributeTechnique = dynamic_cast<const AlbedoDerivedDetailMappingTechnique*>(config.detailMappingTechnique.get()); attributeTechnique)
	{
		ss.setDefine("DETAIL_MAPPING_TECHNIQUE_ALBEDO_DERIVED");
		ss.setDefine("DETAIL_SAMPLER_COUNT", std::to_string(attributeTechnique->albedoDetailMaps.size()));

		ss.setTextureAttributeAndModes(unit, attributeTechnique->noiseMap);
		ss.addUniform(createUniformSampler2d("noiseSampler", unit++));

		osg::Uniform* uniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "albedoDetailSamplers", attributeTechnique->albedoDetailMaps.size());
		for (size_t i = 0; i < attributeTechnique->albedoDetailMaps.size(); ++i)
		{
			ss.setTextureAttributeAndModes(unit, attributeTechnique->albedoDetailMaps[i]);
			uniform->setElement(i, unit++);
		}
		ss.addUniform(uniform);
	}

	HeightMapElevationRerange rerange = getRequiredHeightMapElevationRerange(*config.heightMap->getImage());

	ss.addUniform(new osg::Uniform("heightScale", rerange.x() * 65535.f));
	ss.addUniform(new osg::Uniform("heightOffset", rerange.y()));
	ss.setAttribute(config.program);
	ss.setAttribute(new osg::PatchParameter(4)); 

	ss.addUniform(new osg::Uniform("heightMapUvScale", config.heightMapUvScale));
	ss.addUniform(new osg::Uniform("heightMapUvOffset", config.heightMapUvOffset));

	ss.addUniform(new osg::Uniform("overallAlbedoMapUvScale", config.overallAlbedoMapUvScale));
	ss.addUniform(new osg::Uniform("overallAlbedoMapUvOffset", config.overallAlbedoMapUvOffset));

	ss.addUniform(new osg::Uniform("attributeMapUvScale", config.attributeMapUvScale));
	ss.addUniform(new osg::Uniform("attributeMapUvOffset", config.attributeMapUvOffset));

	ss.setDefine("ACCURATE_LOG_Z");

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

	ss.setAttributeAndModes( polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
#endif
}

Terrain::Terrain(const TerrainConfig& config)
{
	TerrainConfig::FlatTile* flatTile = dynamic_cast<TerrainConfig::FlatTile*>(config.tile.get());
	TerrainConfig::PlanetTile* planetTile = dynamic_cast<TerrainConfig::PlanetTile*>(config.tile.get());

	if (flatTile)
	{
		mNode = createFlatTileGeode(flatTile->size);
	}
	else if (planetTile)
	{
		osg::Vec3d tilePosition = llaToGeocentric(planetTile->latLonBounds.center(), 0, planetTile->planetRadius);
		mNode = createPlanetTileGeode(tilePosition, planetTile->latLonBounds, planetTile->planetRadius, Quads);
	}
	else
	{
		assert(!"Invalid Tile type");
	}

	setupTerrainStateSet(*mNode->getOrCreateStateSet(), config);
	mTransform->addChild(mNode);
}

Terrain::~Terrain()
{
	mTransform->removeChild(mNode);
}

} // namespace vis
} // namespace skybolt
