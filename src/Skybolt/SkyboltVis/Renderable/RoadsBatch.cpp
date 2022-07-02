/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RoadsBatch.h"
#include "OsgGeometryHelpers.h"
#include "OsgImageHelpers.h"
#include "OsgStateSetHelpers.h"
#include "SkyboltVis/earcutOsg.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include <SkyboltVis/VisibilityCategory.h>
#include "SkyboltVis/Renderable/Planet/Terrain.h"
#include <SkyboltCommon/Math/IntersectionUtility.h>

#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgUtil/Tessellator>

#include <optional>


float roadTextureLengthMeters = 10.f;
float roadBorderSizeMeters = 0.5;
float roadHeightAboveTerrain = 0.0f;
int numLanesInTexture = 2;

using namespace skybolt;

namespace skybolt {
namespace vis {

inline osg::Vec2f toVec2f(const osg::Vec3f& v)
{
	return osg::Vec2f(v.x(), v.y());
}

//! @return normal which is scaled to give segments of unit half-width
static osg::Vec2f getSegmentNormal(const std::optional<osg::Vec3f>& prevPoint, const osg::Vec3f& currentPoint, const std::optional<osg::Vec3f>& nextPoint)
{
	assert(prevPoint.has_value() || nextPoint.has_value());

	osg::Vec2f normal, p0, p1;
	if (!prevPoint.has_value())
	{
		p0 = toVec2f(currentPoint);
		p1 = toVec2f(*nextPoint);

		osg::Vec2f dir = p1 - p0;
		dir.normalize();
		normal = osg::Vec2f(dir.y(), -dir.x());
	}
	else if (!nextPoint.has_value())
	{
		p0 = toVec2f(*prevPoint);
		p1 = toVec2f(currentPoint);

		osg::Vec2f dir = p1 - p0;
		dir.normalize();
		normal = osg::Vec2f(dir.y(), -dir.x());
	}
	else
	{
		osg::Vec2f p2;

		p0 = toVec2f(*prevPoint);
		p1 = toVec2f(currentPoint);
		p2 = toVec2f(*nextPoint);

		osg::Vec2f dir0 = p1 - p0;
		dir0.normalize();

		osg::Vec2f dir1 = p2 - p1;
		dir1.normalize();

		osg::Vec2f n0(dir0.y(), -dir0.x());
		osg::Vec2f n1(dir1.y(), -dir1.x());

		normal = osg::Vec2f(n0 + n1);
		normal.normalize();

		float theta = std::acos(std::min(1.f, n0 * n1)) * 0.5f;
		normal *= 1.0f / std::max(0.5f, std::cos(theta));
	}
	return normal;
}

static void createRoad(const Road& road, osg::Vec3Array* posBuffer, osg::Vec3Array* normalBuffer, osg::Vec3Array* uvBuffer, osg::UIntArray* indexBuffer)
{
	assert(road.points.size() > 1);

	size_t firstVertIndex = posBuffer->size();

	float t = 0;

	float width = road.width + roadBorderSizeMeters * 2.0f;
	float halfWidth = width * 0.5f;

	int addedPoints = 0;
	for (int i = 0; i < road.points.size(); ++i)
	{
		// Get previous point
		std::optional<osg::Vec3f> prevPoint;
		if (i > 0)
		{
			prevPoint = road.points[i - 1];

			// Increase road length
			float segmentLength = toVec2f(road.points[i] - *prevPoint).length();
			t += segmentLength;
		}
		else if (road.endLaneCounts[0] > 0)
		{
			prevPoint = road.endControlPoints[0];
		}

		// Get next point
		std::optional<osg::Vec3f> nextPoint;
		if (i < road.points.size() - 1)
		{
			nextPoint = road.points[i + 1];
		}
		else if (road.endLaneCounts[1] > 0)
		{
			nextPoint = road.endControlPoints[1];
		}

		// Add geometry to buffers
		osg::Vec3f point = road.points[i];
		osg::Vec2f normal = getSegmentNormal(prevPoint, point, nextPoint);

		float effectiveHalfWidth;
		int effectiveLaneCount;
		if (i == 0 && road.endLaneCounts[0] > 0)
		{
			effectiveLaneCount = road.endLaneCounts[0];
			effectiveHalfWidth = float(effectiveLaneCount) / float(road.laneCount) * halfWidth;
		}
		else if ((i == road.points.size() - 1) && road.endLaneCounts[1] > 0)
		{
			effectiveLaneCount = road.endLaneCounts[1];
			effectiveHalfWidth = float(effectiveLaneCount) / float(road.laneCount) * halfWidth;
		}
		else
		{
			effectiveLaneCount = road.laneCount;
			effectiveHalfWidth = halfWidth;
		}

		posBuffer->push_back(point + osg::Vec3(normal * effectiveHalfWidth, -roadHeightAboveTerrain));
		posBuffer->push_back(point - osg::Vec3(normal * effectiveHalfWidth, -roadHeightAboveTerrain));
		
		normalBuffer->push_back(osg::Vec3(0, 0, -1)); // TODO: get terrain normal
		normalBuffer->push_back(osg::Vec3(0, 0, -1));

		float scaleX = (float)effectiveLaneCount / (float)numLanesInTexture;
		float scaleY = t / roadTextureLengthMeters;

		uvBuffer->push_back(osg::Vec3f(-roadBorderSizeMeters / road.width * scaleX, scaleY, 0.0));
		uvBuffer->push_back(osg::Vec3f((1.0 + roadBorderSizeMeters / road.width) * scaleX, scaleY, 1.0));

		++addedPoints;
	}

	for (int i = 0; i < addedPoints-1; ++i)
	{
		int j = firstVertIndex + i*2;
		indexBuffer->push_back(j);
		indexBuffer->push_back(j + 1);
		indexBuffer->push_back(j + 2);
		indexBuffer->push_back(j + 1);
		indexBuffer->push_back(j + 3);
		indexBuffer->push_back(j + 2);
	}
}

static osg::Geode* createRoads(const Roads& roads)
{
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::Vec3Array* normalBuffer = new osg::Vec3Array();
	osg::Vec3Array* uvBuffer = new osg::Vec3Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();

	for (size_t i = 0; i < roads.size(); ++i)
	{
		createRoad(roads[i], posBuffer, normalBuffer, uvBuffer, indexBuffer);
	}

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    geometry->setVertexArray(posBuffer);
	geometry->setNormalArray(normalBuffer);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setTexCoordArray(0, uvBuffer); 
	configureDrawable(*geometry);

    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(osg::BoundingBox())); // TODO: set bounds
    geode->addDrawable(geometry);
    return geode;
}

static osg::ref_ptr<osg::Texture2D> createRoadTexture(const std::string& filename)
{
	osg::ref_ptr<osg::Texture2D> texture = vis::createTilingSrgbTexture(readImageWithCorrectOrientation(filename));
	texture->setMaxAnisotropy(8.f);
	return texture;
}

static osg::ref_ptr<osg::StateSet> createStateSet(const osg::ref_ptr<osg::Program>& program,  const RoadsBatch::Uniforms& uniforms)
{
	osg::ref_ptr<osg::StateSet> ss(new osg::StateSet);

	ss->setAttribute(program);
	ss->addUniform(uniforms.modelMatrix);
	ss->setDefine("ENABLE_DEPTH_OFFSET");
	ss->setDefine("ACCURATE_LOG_Z"); // enabled because geometry is sparsely tessellated
	ss->addUniform(new osg::Uniform("depthOffset", -0.01f));

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	ss->setAttributeAndModes(depth);

	return ss;
}

RoadsBatch::RoadsBatch(const Roads& roads, const osg::ref_ptr<osg::Program>& program)
{
	osg::Geode* geode = createRoads(roads);
	mUniforms.modelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());

	auto stateSet = createStateSet(program, mUniforms);
	{
		int unit = 0;
		stateSet->setTextureAttributeAndModes(unit, createRoadTexture("Environment/Road/Asphalt010_2K_Road_Color.jpg"));
		stateSet->addUniform(createUniformSampler2d("albedoSampler", unit++));
		stateSet->setTextureAttributeAndModes(unit, createRoadTexture("Environment/Road/RoadMarkings.png"));
		stateSet->addUniform(createUniformSampler2d("markingsSampler", unit++));
	}

	geode->setStateSet(stateSet);
	mTransform->addChild(geode);
}

static const float pavementTextureScale = 0.02f;

static void createRegion(const PolyRegion& region, osg::Vec3Array* posBuffer, osg::Vec3Array* normalBuffer, osg::Vec2Array* uvBuffer, osg::UIntArray* indexBuffer)
{
	assert(region.points.size() >= 3);
	size_t indexoffset = posBuffer->size();

	std::vector<osg::Vec3> points = region.points;
	std::vector<std::vector<osg::Vec3f>> polygon = { points };

	std::vector<int> indices = mapbox::earcut<int>(polygon);

	for (int i = 0; i < region.points.size(); ++i)
	{	
		posBuffer->push_back(region.points[i] + osg::Vec3(0,0, -roadHeightAboveTerrain));
		normalBuffer->push_back(osg::Vec3(0, 0, -1)); // TODO: get from terrain normal
		uvBuffer->push_back(toVec2f(region.points[i]) * pavementTextureScale);
	}

	for (int i = 0; i < indices.size(); i += 3)
	{
		indexBuffer->push_back(indexoffset + indices[i]);
		indexBuffer->push_back(indexoffset + indices[i + 2]); // reverse winding
		indexBuffer->push_back(indexoffset + indices[i + 1]);
	}
}

static osg::Geode* createRegions(const PolyRegions& regions)
{
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::Vec3Array* normalBuffer = new osg::Vec3Array();
	osg::Vec2Array* uvBuffer = new osg::Vec2Array();
	osg::ref_ptr<osg::UIntArray> indexBuffer = new osg::UIntArray();

    osg::Geometry* geometry = new osg::Geometry();
	int firstVert = 0;
	for (size_t i = 0; i < regions.size(); ++i)
	{
		createRegion(regions[i], posBuffer, normalBuffer, uvBuffer, indexBuffer);
	}
	geometry->setVertexArray(posBuffer);
	geometry->setNormalArray(normalBuffer);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	geometry->setTexCoordArray(0, uvBuffer);
	configureDrawable(*geometry);

	geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));

    osg::Geode* geode = new osg::Geode();
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(osg::BoundingBox())); // TODO: set bounds
    geode->addDrawable(geometry);
    return geode;
}

RoadsBatch::RoadsBatch(const PolyRegions& regions, const osg::ref_ptr<osg::Program>& program)
{
	osg::Geode* geode = createRegions(regions);
	mUniforms.modelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());

	auto stateSet = createStateSet(program, mUniforms);
	{
		int unit = 0;
		stateSet->setTextureAttributeAndModes(unit, createRoadTexture("Environment/Concrete/Concrete010_2K_Color.jpg"));
		stateSet->addUniform(createUniformSampler2d("albedoSampler", ++unit));
	}

	geode->setStateSet(stateSet);
	mTransform->addChild(geode);
}

void RoadsBatch::updatePreRender(const CameraRenderContext& context)
{
	mUniforms.modelMatrix->set(mTransform->getWorldMatrices().front());
}

glm::vec2 toGlmVec2(const osg::Vec2f& v)
{
	return glm::vec2(v.x(), v.y());
}

glm::vec2 swapComponents(const glm::vec2& v)
{
	return glm::vec2(v.y, v.x);
}

Grid toGrid(const osg::Image& image)
{
	Grid grid;
	grid.origin = glm::vec2(0, 0);
	grid.cellSize = glm::vec2(1, 1);
	grid.countX = image.s();
	grid.countY = image.t();

	return grid;
}

void RoadsBatch::replaceAttribute(const Roads& roads, osg::Image& image, const Box2f& imageWorldBounds, int attributeToReplace, int attributeToReplaceWith)
{
	Grid grid = toGrid(image);

	glm::vec2 minimumBound = toGlmVec2(imageWorldBounds.minimum);
	glm::vec2 scale = glm::vec2(image.s(), image.t()) / (toGlmVec2(imageWorldBounds.maximum) - minimumBound);

	std::vector<glm::ivec2> cells;
	for (const Road& road : roads)
	{
		int segmentCount = (int)road.points.size() - 1;
		for (int i = 0; i < segmentCount; ++i)
		{
			glm::vec2 p0 = toGlmVec2(toVec2f(road.points[i]));
			glm::vec2 p1 = toGlmVec2(toVec2f(road.points[i + 1]));

			// Transform points into image space
			p0 = swapComponents((p0 - minimumBound) * scale);
			p1 = swapComponents((p1 - minimumBound) * scale);

			// Find cells intersecting road
			glm::vec2 dir = p1 - p0;
			float length = glm::length(dir);
			if (length < 1e-8f)
				continue;

			dir /= length;

			intersectRayGrid(grid, p0, dir, length, cells);

			// Replace attributes in cells
			for (const glm::ivec2 p : cells)
			{
				char& v = ((char*)image.data())[p.y * image.s() + p.x];
				if (v == attributeToReplace)
					v = attributeToReplaceWith;
			}

			cells.clear();
		}
	}
}

} // namespace vis
} // namespace skybolt
