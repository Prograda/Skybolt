/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TerrainCollisionShape.h"
#include "AltitudeProvider.h"
#include "BulletTypeConversion.h"
#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include <assert.h>

namespace skybolt {
namespace sim {

TerrainCollisionShape::TerrainCollisionShape(const std::shared_ptr<AltitudeProvider>& altitudeProvider, double planetRadius, double maxPlanetRadius) :
	mAltitudeProvider(altitudeProvider),
	mPlanetRadius(planetRadius),
	mMaxPlanetRadius(maxPlanetRadius),
	mLocalScaling(1,1,1)
{
	assert(mAltitudeProvider);

	m_shapeType = CUSTOM_CONCAVE_SHAPE_TYPE;
}

void TerrainCollisionShape::processAllTriangles(btTriangleCallback *callback, const btVector3 &aabbMin, const btVector3 &aabbMax) const
{
	Vector3 center = toGlmDvec3((aabbMin + aabbMax) * 0.5);
	if (glm::dot(center, center) <= 1e-8)
	{
		return;
	}

	double radius = (aabbMax - aabbMin).length();

	sim::LatLon latLon = geocentricToLatLon(center);

	double altitude = mAltitudeProvider->get(latLon);

	Vector3 normal = glm::normalize(center);
	Vector3 tangent, bitangent;
	getOrthonormalBasis(normal, tangent, bitangent);

	Vector3 planeCenter = normal * (mPlanetRadius + altitude);

	Vector3 corner[4];
	corner[0] = planeCenter + (tangent + bitangent) * radius;
	corner[1] = planeCenter + (tangent - bitangent) * radius;
	corner[2] = planeCenter + (-tangent - bitangent) * radius;
	corner[3] = planeCenter + (-tangent + bitangent) * radius;

	btVector3 t0[3];
	t0[0] = toBtVector3(corner[0]);
	t0[1] = toBtVector3(corner[1]);
	t0[2] = toBtVector3(corner[2]);

	btVector3 t1[3];
	t1[0] = toBtVector3(corner[0]);
	t1[1] = toBtVector3(corner[2]);
	t1[2] = toBtVector3(corner[3]);

	int part = 0;
	int index = 0;
	callback->processTriangle(t0, part, index);
	callback->processTriangle(t1, part, index);
}

void TerrainCollisionShape::getAabb(const btTransform &transform, btVector3 &aabbMin, btVector3 &aabbMax) const
{
	aabbMin = btVector3(-mMaxPlanetRadius, -mMaxPlanetRadius, -mMaxPlanetRadius);
	aabbMax = btVector3(mMaxPlanetRadius, mMaxPlanetRadius, mMaxPlanetRadius);
}

void TerrainCollisionShape::calculateLocalInertia(btScalar mass, btVector3 &inertia) const
{
	inertia.setValue(btScalar(0.), btScalar(0.), btScalar(0.));
}

} // namespace sim
} // namespace skybolt