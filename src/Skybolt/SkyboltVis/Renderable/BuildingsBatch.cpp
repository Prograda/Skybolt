/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BuildingsBatch.h"
#include "earcutOsg.h"
#include "OsgImageHelpers.h"
#include "OsgStateSetHelpers.h"

#include <SkyboltCommon/Random.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture2DArray>

const int horizontalWindowsInTexture = 10;
const float buildingLevelHeight = 3.9f;
const float roofTextureWorldSize = 20;
const float roofUvScale = 1.0f / roofTextureWorldSize;

namespace skybolt {
namespace vis {

struct FacadeTexture
{
	std::string imageFilename;
	int buildingLevelsInTexture;
	int horizontalSectionsInTexture;
};

std::vector<FacadeTexture> facadeTextures = {
	{ "Environment/Facade/Facade001_2K_Color.jpg", 10, 16 },
	{ "Environment/Facade/Facade006_2K_Color.jpg", 8, 14 }
};

struct RoofTexture
{
	std::string imageFilename;
};

std::vector<RoofTexture> roofTextures = {
	{ "Environment/Concrete/Concrete006_2K_Color_Brightened.jpg" },
	{ "Environment/Concrete/Concrete010_2K_Color.jpg" },
	{ "Environment/Concrete/Concrete017_2K_Color.jpg" }
};

class BoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback // TODO
{
	osg::BoundingBox computeBound(const osg::Drawable & drawable)
	{
		return osg::BoundingBox(osg::Vec3f(-FLT_MAX, -FLT_MAX, 0), osg::Vec3f(FLT_MAX, FLT_MAX, 0));
	}
};

inline osg::Vec2f toVec2f(const osg::Vec3f& v)
{
	return osg::Vec2f(v.x(), v.y());
}

static osg::Vec2f getNormal(const std::vector<osg::Vec3f>& points, int i)
{
	osg::Vec2f p0 = toVec2f(points[i]);
	osg::Vec2f p1 = toVec2f(points[(i + 1) % points.size()]);

	osg::Vec2f dir = p1 - p0;
	dir.normalize();
	return osg::Vec2f(dir.y(), -dir.x());
}

static void createBuilding(const Building& building, const FacadeTexture& facade, int facadeIndex, osg::Vec3Array* posBuffer, osg::Vec3Array* normalBuffer, osg::Vec4Array* uvBuffer, osg::UIntArray* indexBuffer, int buildingIndex)
{
	assert(building.points.size() > 1);

	size_t index = posBuffer->size();
	int pointCount = (int)building.points.size();
	for (int i = 0; i < pointCount; ++i)
	{	
		int nextIndex = (i + 1) % pointCount;

		// Quad vertices
		osg::Vec3f v0 = building.points[i];
		osg::Vec3f v1 = building.points[nextIndex];

		posBuffer->push_back(v0);
		posBuffer->push_back(v1);

		v0.z() -= building.height;
		v1.z() -= building.height;
		posBuffer->push_back(v1);
		posBuffer->push_back(v0);

		// Normals
		osg::Vec2f normal = getNormal(building.points, i);
		for (int n = 0; n < 4; ++n)
		{
			normalBuffer->push_back(osg::Vec3f(normal.x(), normal.y(), 0.f));
		}

		// UVs
		int levels = ceilf(building.height / buildingLevelHeight);

		// calculate vertical repeats so that texture is not cut off vertically mid level within the texture
		float textureWorldHeight = facade.buildingLevelsInTexture * buildingLevelHeight;
		float textureRepeatsY = float(levels) / facade.buildingLevelsInTexture;

		// calcualte horizontal repeats so that texture is not cut off horizontally mid 'section' (e.g a window) within the texture
		float horizontalWidth = toVec2f(v1 - v0).length();
		float textureWorldWidth = textureWorldHeight; // assume texture is square
		float textureRepeatsX = horizontalWidth / textureWorldWidth;
		textureRepeatsX = std::ceil(textureRepeatsX * facade.horizontalSectionsInTexture) / (float)facade.horizontalSectionsInTexture;

		uvBuffer->push_back(osg::Vec4f(i * textureRepeatsX, 0, facadeIndex, buildingIndex));
		uvBuffer->push_back(osg::Vec4f(nextIndex * textureRepeatsX, 0, facadeIndex, buildingIndex));
		uvBuffer->push_back(osg::Vec4f(nextIndex * textureRepeatsX, textureRepeatsY, facadeIndex, buildingIndex));
		uvBuffer->push_back(osg::Vec4f(i * textureRepeatsX, textureRepeatsY, facadeIndex, buildingIndex));

		// Indicies
		indexBuffer->push_back(index);
		indexBuffer->push_back(index + 3);
		indexBuffer->push_back(index + 2);
		indexBuffer->push_back(index);
		indexBuffer->push_back(index + 2);
		indexBuffer->push_back(index + 1);
		index += 4;
	}
}

static void createBuildingRoof(const Building& building, int textureIndex, osg::Vec3Array* posBuffer, osg::Vec3Array* normalBuffer, osg::Vec4Array* uvBuffer, osg::UIntArray* indexBuffer, int buildingIndex)
{
	assert(building.points.size() > 1);

	std::vector<std::vector<osg::Vec3f>> polygon;
	polygon.push_back(building.points);
	std::reverse(polygon.front().begin(), polygon.front().end()); // reverse winding for earcut algorithm

	// Run tessellation
	// Returns array of indices that refer to the vertices of the input polygon.
	// Three subsequent indices form a triangle.
	using N = uint32_t;
	std::vector<N> indices = mapbox::earcut<N>(polygon);

	size_t index = posBuffer->size();
	int pointCount = (int)building.points.size();
	for (int i = 0; i < pointCount; ++i)
	{
		// Quad vertices
		osg::Vec3f v0 = building.points[i];
		v0.z() -= building.height;
		posBuffer->push_back(v0);

		// Normals
		osg::Vec2f normal = getNormal(building.points, i);
		normalBuffer->push_back(osg::Vec3f(0, 0, -1.f));

		// UVs
		uvBuffer->push_back(osg::Vec4f(v0.x() * roofUvScale, v0.y() * roofUvScale, textureIndex, buildingIndex));
	}

	for (size_t i = 0; i < indices.size(); ++i)
	{
		indexBuffer->push_back(index + indices[i]);
	}
}

static osg::Geode* createBuildings(const Buildings& buildings)
{
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::Vec3Array* normalBuffer = new osg::Vec3Array();
	osg::Vec4Array* uvBuffer = new osg::Vec4Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();

	skybolt::Random random(0);

	for (size_t i = 0; i < buildings.size(); ++i)
	{
		const Building& building = buildings[i];

		const osg::Vec3f& pos = building.points.front();

		int facadeIndex = i % facadeTextures.size();

		createBuilding(building, facadeTextures[facadeIndex], facadeIndex, posBuffer, normalBuffer, uvBuffer, indexBuffer, i);

		int roofTextureIndex = random.getInt(facadeTextures.size(), facadeTextures.size() + roofTextures.size() - 1);
		createBuildingRoof(building, roofTextureIndex, posBuffer, normalBuffer, uvBuffer, indexBuffer, i);
	}

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    geometry->setVertexArray(posBuffer);
	geometry->setNormalArray(normalBuffer);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	geometry->setTexCoordArray(0, uvBuffer); 
	geometry->setUseDisplayList(false); 
    geometry->setUseVertexBufferObjects(true); 
	geometry->setUseVertexArrayObject(true);

    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
	geometry->setComputeBoundingBoxCallback(osg::ref_ptr<BoundingBoxCallback>(new BoundingBoxCallback));
    geode->addDrawable(geometry);
    return geode;
}

static osg::ref_ptr<osg::Texture2DArray> createTextureArray()
{
	osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
	texture->setInternalFormat(GL_SRGB8);
	texture->setSourceFormat(GL_RGB);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setUseHardwareMipMapGeneration(true);

	int i = 0;
	for (const FacadeTexture& facade : facadeTextures)
	{
		osg::Image* image = readImageWithCorrectOrientation(facade.imageFilename);
		image->setInternalTextureFormat(toSrgbInternalFormat(image->getInternalTextureFormat()));
		texture->setImage(i, image);
		++i;
	}

	for (const RoofTexture& roof : roofTextures)
	{
		osg::Image* image = readImageWithCorrectOrientation(roof.imageFilename);
		image->setInternalTextureFormat(toSrgbInternalFormat(image->getInternalTextureFormat()));
		texture->setImage(i, image);
		++i;
	}
	return texture;
}

static osg::ref_ptr<osg::Texture2DArray> getTextureArray()
{
	static osg::ref_ptr<osg::Texture2DArray> texture = createTextureArray();
	return texture;
}

static osg::ref_ptr<osg::StateSet> createStateSet(const osg::ref_ptr<osg::Program>& program, osg::ref_ptr<osg::Texture2DArray> texture, const BuildingsBatch::Uniforms& uniforms, const ShadowMaps& shadowMaps)
{
	osg::ref_ptr<osg::StateSet> ss(new osg::StateSet);
	ss->setTextureAttributeAndModes(0, texture);
	ss->addUniform(createUniformSampler2dArray("albedoSampler", 0));

	ss->setAttribute(program);
	ss->addUniform(uniforms.modelMatrix);

	addShadowMapsToStateSet(shadowMaps, *ss, 1);

	return ss;
}

BuildingsBatch::BuildingsBatch(const Buildings& buildings, const osg::ref_ptr<osg::Program>& program, const ShadowMaps& shadowMaps)
{
	osg::Geode* geode = createBuildings(buildings);
	mUniforms.modelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());
	geode->setStateSet(createStateSet(program, getTextureArray(), mUniforms, shadowMaps));
	mTransform->addChild(geode);
}

void BuildingsBatch::updatePreRender(const RenderContext& context)
{
	mUniforms.modelMatrix->set(mTransform->getWorldMatrices().front());
}

} // namespace vis
} // namespace skybolt
