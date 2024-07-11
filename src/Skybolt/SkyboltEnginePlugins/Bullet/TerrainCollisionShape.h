/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/LatLon.h"
#include <BulletCollision/CollisionShapes/btConcaveShape.h>
#include <memory>

namespace skybolt {
namespace sim {

class AltitudeProvider;

class TerrainCollisionShape : public btConcaveShape
{
public:
	TerrainCollisionShape(const std::shared_ptr<AltitudeProvider>& elevationProvider, double planetRadius, double maxPlanetRadius);

	void processAllTriangles(btTriangleCallback *callback, const btVector3 &aabbMin, const btVector3 &aabbMax) const override;

	void btCollisionShape::getAabb(const btTransform &transform, btVector3 & aabbMin, btVector3 &aabbMax) const override;

	void btCollisionShape::setLocalScaling(const btVector3 &scaling) override {}

	const btVector3 &btCollisionShape::getLocalScaling() const override { return mLocalScaling; }

	void btCollisionShape::calculateLocalInertia(btScalar mass, btVector3 &inertia) const override;

	const char *btCollisionShape::getName() const override { return "TerrainCollisionShape"; }

private:
	double getAltitude(const Vector3& position) const;

private:
	std::shared_ptr<AltitudeProvider> mAltitudeProvider;
	double mPlanetRadius; //!< Reference radius for altitude = 0
	double mMaxPlanetRadius;
	btVector3 mLocalScaling;
};

} // namespace sim
} // namespace skybolt