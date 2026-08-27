/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/CollisionBody.h"
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
class DynamicBodyComponent : public Component, public CollisionBody
{
public:
	virtual void setMass(double mass) = 0;
	virtual double getMass() const = 0;

	virtual Vector3 getMomentOfInertia() const = 0;
	virtual void setMomentOfInertia(const Vector3& momentOfInertia) = 0;
	
	//! Gets the center of mass relative to the body position in body axes
	virtual Vector3 getCenterOfMass() const = 0;

	//! Sets the center of mass relative to the body position in body axes
	virtual void setCenterOfMass(const Vector3& relPosition) = 0;

	//! Apply force at center of mass during the next dynamics substep. Force is in world axes.
	virtual void applyCentralForce(const Vector3& force) = 0;

	//! Apply force at relative position during the next dynamics substep. 
	//! Force and relPosition are in world axes
	virtual void applyForce(const Vector3& force, const Vector3& relPosition) = 0;

	//! Apply torque during the next dynamics substep. Torque is in world axes.
	virtual void applyTorque(const Vector3& torque) = 0;

	//! @returns forces applied in the most recent dynamics substep.
	//! Used for visualisation purposes only; this should not affect the simulation.
	const std::vector<AppliedForce>& getForcesAppliedInLastSubstep() const { return mForcesAppliedInLastSubstep; }

	virtual void setLinearDamping(double damping) = 0;

	virtual void setAngularDamping(double damping) = 0;

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(DynamicBodyComponent), typeid(CollisionBody)};
	}

protected:
	std::vector<AppliedForce> mForcesAppliedInLastSubstep;
};

SKYBOLT_REFLECT_EXTERN(DynamicBodyComponent)

} // namespace sim
} // namespace skybolt