/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Terrain.h"
#include "OsgGeocentric.h"
#include "OsgGeometryFactory.h"
#include "OsgImageHelpers.h"
#include "OsgStateSetHelpers.h"
#include "PlanetTileGeometry.h"

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

// Creates a 'textureizer map' which is a detail map that can be added
// to a lower detail base map.
// To produce a texturizer map, we take an original detail map, find the average,
// subtract the average, and add 0.5 to all channels. The result is a map that contains
// just the offsets to apply the original texture to a base map while preserving the average
// color of the base map.
// TODO: need to test this more to see if it is worth using. Currently unused.
static void convertSrgbToTexturizerMap(osg::Image& image)
{
	osg::Vec4f averageLinearSpace = srgbToLinear(averageSrgbColor(image));

	osg::Vec4f color;
	size_t pixels = 0;
	for (size_t t = 0; t < image.t(); ++t)
	{
		for (size_t s = 0; s < image.s(); ++s)
		{
			osg::Vec4f c = srgbToLinear(image.getColor(s, t));
			c = c - averageLinearSpace + osg::Vec4f(0.5, 0.5, 0.5, 0.5);
			image.setColor(linearToSrgb(c), s, t);
		}
	}
}

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
	geometry->setUseDisplayList(false); 
    geometry->setUseVertexBufferObjects(true); 
	geometry->setUseVertexArrayObject(true);

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

	ss.setTextureAttributeAndModes(unit, config.overallAlbedoMap);
	ss.addUniform(createUniformSampler2d("overallAlbedoSampler", unit++));

	if (config.detailMaps)
	{
		ss.setDefine("ENABLE_DETAIL_ALBEDO_TEXTURES");
		const auto& detailMaps = *config.detailMaps;
		{
			ss.setTextureAttributeAndModes(unit, detailMaps.attributeMap);
			ss.addUniform(createUniformSampler2d("attributeSampler", unit++));
		}

		osg::Uniform* uniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "albedoDetailSamplers", detailMaps.albedoDetailMaps.size());

		for (size_t i = 0; i < detailMaps.albedoDetailMaps.size(); ++i)
		{
			ss.setTextureAttributeAndModes(unit, detailMaps.albedoDetailMaps[i]);
			uniform->setElement(i, unit++);
		}
		ss.addUniform(uniform);

		//ss.setTextureAttributeAndModes(unit, detailMaps.noiseMap);
		//ss.addUniform(createUniformSampler2d("noiseSampler", unit++));
	}

	addShadowMapsToStateSet(config.shadowMaps, ss, unit);
	unit += config.shadowMaps.size();

    osg::Uniform* heightScaleUniform = new osg::Uniform("heightScale", config.heightScale);
    ss.addUniform(heightScaleUniform);
	ss.setAttribute(config.program);
	ss.setAttribute(new osg::PatchParameter(4)); 

	ss.addUniform(new osg::Uniform("heightMapUvScale", config.heightMapUvScale));
	ss.addUniform(new osg::Uniform("heightMapUvOffset", config.heightMapUvOffset));

	ss.addUniform(new osg::Uniform("overallAlbedoMapUvScale", config.overallAlbedoMapUvScale));
	ss.addUniform(new osg::Uniform("overallAlbedoMapUvOffset", config.overallAlbedoMapUvOffset));

	ss.addUniform(new osg::Uniform("attributeMapUvScale", config.attributeMapUvScale));
	ss.addUniform(new osg::Uniform("attributeMapUvOffset", config.attributeMapUvOffset));

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

	ss.setAttributeAndModes( polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
#endif
}

class BoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
	osg::BoundingBox computeBound(const osg::Drawable & drawable)
	{
		// TODO: use actual bounds
		return osg::BoundingBox(osg::Vec3f(-FLT_MAX, -FLT_MAX, 0), osg::Vec3f(FLT_MAX, FLT_MAX, 0));
	}
};

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

void Terrain::updatePreRender(const RenderContext& context)
{
}

} // namespace vis
} // namespace skybolt
