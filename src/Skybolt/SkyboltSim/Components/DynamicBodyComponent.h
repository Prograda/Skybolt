/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/Positionable.h"
#include <vector>
#include <map>
#include <set>

class btTypedConstraint;
class btGeneric6DofConstraint;

namespace skybolt {
namespace sim {

struct AppliedForce
{
	Vector3 positionRelBody; //!< Position of force relative to body position in world axes
	Vector3 force;
};

// Emits CollisionEvent upon collision
class DynamicBodyComponent : public Component
{
public:
	virtual void setLinearVelocity(const Vector3& v) = 0;
	virtual Vector3 getLinearVelocity() const = 0;
	
	//! Angular velocity is in world space
	virtual void setAngularVelocity(const Vector3& v) = 0;
	virtual Vector3 getAngularVelocity() const = 0;

	virtual void setMass(Real mass) = 0;
	virtual Real getMass() const = 0;

	//! relPosition is in body axes
	virtual void setCenterOfMass(const Vector3& relPosition) = 0;

	//! Apply force at center of mass during the next dynamics substep. Force is in world axes.
	virtual void applyCentralForce(const Vector3& force) = 0;

	//! Apply force at relative position during the next dynamics substep. 
	//! Force and relPosition are in world axes
	virtual void applyForce(const Vector3& force, const Vector3& relPosition) = 0;

	//! Apply torque during the next dynamics substep. Torque is in world axes.
	virtual void applyTorque(const Vector3& torque) = 0;

	//! @returns forces applied during the most recent dynamics substep.
	//! Used for visualisation purposes.
	const std::vector<AppliedForce>& getForces() const { return mForces; }

	virtual void setCollisionsEnabled(bool enabled) = 0;

protected:
	std::vector<AppliedForce> mForces;
};

} // namespace sim
} // namespace skybolt