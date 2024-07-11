/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetTileGeometry.h"
#include "OsgGeometryHelpers.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Geode>
#include <assert.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

osg::PrimitiveSet::Mode toOsgPrimitiveType(PrimitiveType type)
{
	switch (type)
	{
	case Triangles:
		return osg::PrimitiveSet::TRIANGLES;
	case Quads:
		return osg::PrimitiveSet::PATCHES;
	default:
		assert(!"Enum value not implemented");
		return osg::PrimitiveSet::PATCHES;
	}
}

osg::Geometry* createPlanetTileGeometry(const osg::Vec3d& tileCenter, const Box2d& latLonBounds,
	double radius, float skirtLength, PrimitiveType type)
{
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();
	int segmentCountX = 64;
	int segmentCountY = 64;

	createPlaneBuffers(*posBuffer, *indexBuffer, osg::Vec2f(0,0), osg::Vec2f(1,1), segmentCountX, segmentCountY, type);

	osg::Vec2Array* uvBuffer = new osg::Vec2Array();
	uvBuffer->resize(posBuffer->size());

	int countX = segmentCountX + 1;
	int countY = segmentCountY + 1;
	int innerMaxX = segmentCountX - 2;
	int innerMaxY = segmentCountY - 2;

	osg::BoundingBoxf bounds;

	size_t i = 0;
	for (int y = 0; y < countY; ++y)
	{
		for (int x = 0; x < countX; ++x)
		{
			bool skirt = (x == 0 || x == segmentCountX || y == 0 || y == segmentCountY);
			osg::Vec2f uv;
			uv.x() = (float)math::clamp(x - 1, 0, innerMaxX) / (float)innerMaxX;
			uv.y() = (float)math::clamp(y - 1, 0, innerMaxY) / (float)innerMaxY;

			osg::Vec2d latLon = latLonBounds.getPointFromNormalizedCoord(math::vec2SwapComponents(uv));

			double effectiveRadius = skirt ? (radius - double(skirtLength)) : radius;
			osg::Vec3f pos = llaToGeocentric(latLon, 0, effectiveRadius) - tileCenter;
			posBuffer->at(i) = pos;
			uvBuffer->at(i) = uv;
			++i;

			// Add point to bounding box, with some vertical padding to ensure vertical bounds are large enough to account for height in heightmap.
			// TODO: use actual tile vertical bounds.
			pos = llaToGeocentric(latLon, 0, radius - 9000) - tileCenter; // lowest point
			bounds.expandBy(pos);
			pos = llaToGeocentric(latLon, 0, radius + 9000) - tileCenter; // highest point
			bounds.expandBy(pos);
		}
	}

	osg::Geometry *geometry = new osg::Geometry();

	geometry->setVertexArray(posBuffer);
	geometry->setTexCoordArray(0, uvBuffer);
	vis::configureDrawable(*geometry);
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(bounds));

	geometry->addPrimitiveSet(new osg::DrawElementsUInt(toOsgPrimitiveType(type), indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
	return geometry;
}

osg::Geode* createPlanetTileGeode(const osg::Vec3d& tileCenter, const Box2d& latLonBounds, double planetRadius, PrimitiveType type)
{
	float skirtLength = 0.005 * latLonBounds.size().length() * planetRadius; // TODO: tweak
	osg::ref_ptr<osg::Geometry> geometry = createPlanetTileGeometry(tileCenter, latLonBounds, planetRadius, skirtLength, type);

	osg::Geode* geode = new osg::Geode;
	geode->addDrawable(geometry);

	return geode;
}

} // namespace vis
} // namespace skybolt
