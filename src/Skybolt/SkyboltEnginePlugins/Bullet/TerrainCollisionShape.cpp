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

	// m_shapeType = CUSTOM_CONCAVE_SHAPE_TYPE;
	// Work around for Bullet bug where CUSTOM_CONCAVE_SHAPE_TYPE is treated as SDF_SHAPE_PROXYTYPE
	// which causes terrain collisions to fail for some shape types (e.g. Box).
	// See https://github.com/bulletphysics/bullet3/discussions/3744
	m_shapeType = MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE;
}

void TerrainCollisionShape::processAllTriangles(btTriangleCallback *callback, const btVector3 &aabbMin, const btVector3 &aabbMax) const
{
	Vector3 aabbCenter = toGlmDvec3((aabbMin + aabbMax) * 0.5);
	if (glm::dot(aabbCenter, aabbCenter) <= 1e-8)
	{
		return;
	}

	double radius = (aabbMax - aabbMin).length();

	Vector3 normal = glm::normalize(aabbCenter);
	Vector3 tangent, bitangent;
	getOrthonormalBasis(normal, tangent, bitangent);

	Vector3 planetCenter = normal * earthRadius();

	Vector3 p00 = planetCenter + (-tangent - bitangent) * radius;
	Vector3 p10 = planetCenter + (tangent - bitangent) * radius;
	Vector3 p01 = planetCenter + (-tangent + bitangent) * radius;
	Vector3 p11 = planetCenter + (tangent + bitangent) * radius;

	p00 += getAltitude(p00) * normal;
	p10 += getAltitude(p10) * normal;
	p01 += getAltitude(p01) * normal;
	p11 += getAltitude(p11) * normal;

	btVector3 t0[3];
	t0[0] = toBtVector3(p00);
	t0[1] = toBtVector3(p10);
	t0[2] = toBtVector3(p01);

	btVector3 t1[3];
	t1[0] = toBtVector3(p10);
	t1[1] = toBtVector3(p11);
	t1[2] = toBtVector3(p01);

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

double TerrainCollisionShape::getAltitude(const Vector3& position) const
{
	sim::LatLon latLon = geocentricToLatLon(position);
	return mAltitudeProvider->get(latLon);
}

} // namespace sim
} // namespace skybolt