/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgGeometryFactory.h"
#include "OsgGeometryHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

void createPlaneIndicies(osg::UIntArray& indexBuffer, int segmentCountX, int segmentCountY, PrimitiveType type)
{
	// vertices
	int vertexRowCount = segmentCountX + 1;
	int vertexColumnCount = segmentCountY + 1;

	// indices
	int indexCount = segmentCountX * segmentCountY * 4;
	indexBuffer.reserve(indexCount);

	int i = 0;
	for (int y = 0; y < segmentCountY; ++y)
	{
		for (int x = 0; x < segmentCountX; ++x)
		{
			if (type == Quads)
			{
				indexBuffer.push_back(i);
				indexBuffer.push_back(i + vertexRowCount);
				indexBuffer.push_back(i + 1 + vertexRowCount);
				indexBuffer.push_back(i + 1);
			}
			else
			{
				assert(type == Triangles);
				indexBuffer.push_back(i);
				indexBuffer.push_back(i + 1);
				indexBuffer.push_back(i + 1 + vertexRowCount);

				indexBuffer.push_back(i);
				indexBuffer.push_back(i + 1 + vertexRowCount);
				indexBuffer.push_back(i + vertexRowCount);
			}

			++i;
		}
		++i;
	}
}

void createPlaneBuffers(osg::Vec3Array& posBuffer, osg::UIntArray& indexBuffer, const osg::Vec2f& minBound, const osg::Vec2f& size, int segmentCountX, int segmentCountY, PrimitiveType type)
{
	// vertices
	int vertexRowCount = segmentCountX + 1;
	int vertexColumnCount = segmentCountY + 1;
	int vertexCount = vertexRowCount * vertexColumnCount;
	posBuffer.reserve(vertexCount);

	for (int y = 0; y < vertexColumnCount; ++y)
	{
		float sy = (float)y / (float)segmentCountY;
		for (int x = 0; x < vertexRowCount; ++x)
		{
			float sx = (float)x / (float)segmentCountX;

			osg::Vec3f position(minBound.x() + sx * size.x(), minBound.y() + sy * size.y(), 0.0f);
			posBuffer.push_back(position);
		}
	}

	createPlaneIndicies(indexBuffer, segmentCountX, segmentCountY, type);
}

osg::Geometry* createSphere(float radius, unsigned int rings, unsigned int sectors)
{
	osg::Geometry* sphereGeometry = new osg::Geometry;

	osg::Vec3Array* sphereVertices = new osg::Vec3Array;
	osg::Vec2Array* sphereTexCoords = new osg::Vec2Array;

	float const R = 1. / (float)(rings - 1);
	float const S = 1. / (float)(sectors - 1);
	unsigned int r, s;

	// Establish texture coordinates, vertex list, and normals
	for (r = 0; r < rings; r++)
	{
		for (s = 0; s < sectors; s++)
		{
			float const x = cos(2 * math::piF() * s * S) * sin(math::piF() * r * R);
			float const y = sin(2 * math::piF() * s * S) * sin(math::piF() * r * R);
			float const z = cos(math::piF() * r * R);

			sphereTexCoords->push_back(osg::Vec2(s*S, r*R));

			sphereVertices->push_back(osg::Vec3(x * radius,
				y * radius,
				z * radius));
		}
	}

	sphereGeometry->setVertexArray(sphereVertices);
	sphereGeometry->setTexCoordArray(0, sphereTexCoords);

	// Generate quads for each face.  
	osg::UIntArray* indexBuffer = new osg::UIntArray();
	for (r = 0; r < rings - 1; r++)
	{
		for (s = 0; s < sectors - 1; s++)
		{
			// Corners of quads should be in CCW order.
			indexBuffer->push_back((r + 0) * sectors + (s + 0));
			indexBuffer->push_back((r + 0) * sectors + (s + 1));
			indexBuffer->push_back((r + 1) * sectors + (s + 1));

			indexBuffer->push_back((r + 0) * sectors + (s + 0));
			indexBuffer->push_back((r + 1) * sectors + (s + 1));
			indexBuffer->push_back((r + 1) * sectors + (s + 0));
		}
	}
	sphereGeometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
	return sphereGeometry;
}

osg::Geometry* createQuad(const BoundingBox2f& box, QuadUpDirection upDir)
{
	osg::Geometry* quad = new osg::Geometry();

	//quad to create a full screen quad 
	osg::Vec3Array* verts = new osg::Vec3Array(4);
	if (upDir == QuadUpDirectionY)
	{
		(*verts)[0].set(box.xMin(), box.yMin(), 0);
		(*verts)[1].set(box.xMin(), box.yMax(), 0);
		(*verts)[2].set(box.xMax(), box.yMin(), 0);
		(*verts)[3].set(box.xMax(), box.yMax(), 0);
	}
	else
	{
		(*verts)[0].set(0, box.xMin(), -box.yMin());
		(*verts)[1].set(0, box.xMin(), -box.yMax());
		(*verts)[2].set(0, box.xMax(), -box.yMin());
		(*verts)[3].set(0, box.xMax(), -box.yMax());
	}
	quad->setVertexArray(verts);
	quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 4));
	configureDrawable(*quad);

	return quad;
}

osg::Geometry* createQuadWithUvs(const BoundingBox2f& box, QuadUpDirection upDir)
{
	osg::Geometry* quad = new osg::Geometry();

	//quad to create a full screen quad 
	osg::Vec3Array* verts = new osg::Vec3Array(4);
	if (upDir == QuadUpDirectionY)
	{
		(*verts)[0].set(box.xMin(), box.yMin(), 0);
		(*verts)[1].set(box.xMax(), box.yMin(), 0);
		(*verts)[2].set(box.xMin(), box.yMax(), 0);
		(*verts)[3].set(box.xMax(), box.yMax(), 0);
	}
	else
	{
		(*verts)[0].set(0, box.xMin(), -box.yMin());
		(*verts)[1].set(0, box.xMax(), -box.yMin());
		(*verts)[2].set(0, box.xMin(), -box.yMax());
		(*verts)[3].set(0, box.xMax(), -box.yMax());
	}
	quad->setVertexArray(verts);

	osg::Vec2Array* uvs = new osg::Vec2Array(4);
	(*uvs)[0].set(0, 0);
	(*uvs)[1].set(1, 0);
	(*uvs)[2].set(0, 1);
	(*uvs)[3].set(1, 1);
	quad->setTexCoordArray(0, uvs);

	quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 4));
	configureDrawable(*quad);

	return quad;
}

osg::Geometry* createLineBox(const osg::BoundingBox& box)
{
	osg::Geometry* geometry = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array(8);
	for (int i = 0; i < 8; ++i)
	{
		(*vertices)[i] = box.corner(i);
	}

	geometry->setVertexArray(vertices);

	static GLushort indices[] = {
		0,1,
		0,4,
		0,5,
		1,5,

		1,3,
		3,7,
		3,2,
		2,6,

		6,7,
		4,5,
		4,6,
		5,7
	};

	// This time we simply use primitive, and hardwire the number 
	// of coords to use since we know up front,
	geometry->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::LINES, 24, indices));
	configureDrawable(*geometry);

	return geometry;
}

} // namespace vis
} // namespace skybolt
