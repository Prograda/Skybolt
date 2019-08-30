/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "DynamicBodyComponent.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

class DummyDynamicBodyComponent : public DynamicBodyComponent
{
public:
	void setLinearVelocity(const Vector3& v) override {}
	Vector3 getLinearVelocity() const override  { return math::dvec3Zero(); }

	//! Angular velocity is in world space
	void setAngularVelocity(const Vector3& v) {}
	Vector3 getAngularVelocity() const override  { return math::dvec3Zero(); }

	Real getMass() const override  { return 0; }

	void setMass(Real mass) override {}
	void setCenterOfMass(const Vector3& relPosition) override {}

	//! Apply force at center of mass. Force is in world axes.
	void applyCentralForce(const Vector3& force) override {}

	//! Force and relPosition are in world axes
	void applyForce(const Vector3& force, const Vector3& relPosition) override {}

	//! Apply torque. Torque is in world axes.
	void applyTorque(const Vector3& torque) override {}

	virtual void setCollisionsEnabled(bool enabled) override {}
};

} // namespace sim
} // namespace skybolt