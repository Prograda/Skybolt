/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "TriangleMeshShapeData.h"

namespace skybolt {
namespace sim {

TriangleMeshShapeData::TriangleMeshShapeData(float* vertices, int vertexCount, int* indices, int indexCount)
{
	bool use3dBitIndices = (indexCount > 65535);
	mMesh = std::make_unique<btTriangleMesh>(use3dBitIndices);
	mMesh->preallocateVertices(vertexCount);
	mMesh->preallocateIndices(indexCount);

	btVector3 verts[3];
	
	for (int i=0; i<indexCount; i+=3)
	{
		for (int j=0; j<3; j++)
		{
			float* vert = &vertices[3 * indices[i+j]];
			verts[j] = btVector3(*vert, *(vert+1), *(vert+2));
		}
		mMesh->addTriangle(verts[0],verts[1],verts[2],false); // false, don’t remove duplicate vertices
	}
}

TriangleMeshShapeData::~TriangleMeshShapeData() = default;

btBvhTriangleMeshShape* TriangleMeshShapeData::createShape() const
{
	// true for using quantization; true for building the BVH
	return new btBvhTriangleMeshShape(mMesh.get(), true, true);
}

} // namespace sim
} // namespace skybolt