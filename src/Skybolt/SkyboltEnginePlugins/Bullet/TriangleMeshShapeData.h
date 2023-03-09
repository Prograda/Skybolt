/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "btBulletDynamicsCommon.h"

namespace skybolt {
namespace sim {

class TriangleMeshShapeData
{
public:
	
	TriangleMeshShapeData(float* vertices, int vertexCount, int* indices, int indexCount);
	~TriangleMeshShapeData();

	// Caller is responsibile for deleting the shape
	btBvhTriangleMeshShape* createShape() const;

private:
	std::unique_ptr<btTriangleMesh> mMesh;
};

} // namespace sim
} // namespace skybolt