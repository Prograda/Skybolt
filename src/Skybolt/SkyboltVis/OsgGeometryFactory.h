/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OsgBox2.h"
#include <osg/Array>
#include <osg/FrontFace>
#include <osg/Geometry>

namespace skybolt {
namespace vis {

enum PrimitiveType
{
	Triangles,
	Quads
};

enum QuadUpDirection
{
	QuadUpDirectionY,
	QuadUpDirectionNegZ
};

void createPlaneBuffers(osg::Vec3Array& posBuffer, osg::UIntArray& indexBuffer, const osg::Vec2f& minBound, const osg::Vec2f& size, int segmentCountX, int segmentCountY, PrimitiveType type);

enum class SphereFacingMode
{
	OutsideFacing,
	InsideFacing
};

osg::Geometry* createSphere(float radius, unsigned int rings, unsigned int sectors, SphereFacingMode facingMode);

osg::Geometry* createQuad(const BoundingBox2f& box, QuadUpDirection upDir);

osg::Geometry* createQuadWithUvs(const BoundingBox2f& box, QuadUpDirection upDir);

osg::Geometry* createLineBox(const osg::BoundingBox& box);

} // namespace vis
} // namespace skybolt