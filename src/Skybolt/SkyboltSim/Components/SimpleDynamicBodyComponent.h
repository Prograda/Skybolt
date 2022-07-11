/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "DynamicBodyComponent.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

class SimpleDynamicBodyComponent : public DynamicBodyComponent
{
public:
	SimpleDynamicBodyComponent(Node* node, Real mass, const Vector3& momentofInertia);
	void setLinearVelocity(const Vector3& v) override { mLinearVelocity = v; }
	Vector3 getLinearVelocity() const override  { return mLinearVelocity; }

	//! Angular velocity is in world space
	void setAngularVelocity(const Vector3& v) override { mAngularVelocity = v; }
	Vector3 getAngularVelocity() const override  { return mAngularVelocity; }

	Real getMass() const override  { return mMass; }

	void setMass(Real mass) override { mMass = mass; }
	void setCenterOfMass(const Vector3& relPosition) override { mCenterOfMass = relPosition; }

	//! Apply force at center of mass. Force is in world axes.
	void applyCentralForce(const Vector3& force) override;

	//! Force and relPosition are in world axes
	void applyForce(const Vector3& force, const Vector3& relPosition) override;

	//! Apply torque. Torque is in world axes.
	void applyTorque(const Vector3& torque) override;

	void updateDynamicsSubstep(TimeReal dtSubstep) override;

	virtual void setCollisionsEnabled(bool enabled) override {}

	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(DynamicBodyComponent), typeid(SimpleDynamicBodyComponent)};
	}

private:
	Node* mNode;
	Real mMass;
	Vector3 mMomentOfInertia;
	Vector3 mCenterOfMass = math::dvec3Zero();

	Vector3 mLinearVelocity = math::dvec3Zero(); //!< World space
	Vector3 mAngularVelocity = math::dvec3Zero(); //!< World space

	Vector3 mTotalForce = math::dvec3Zero(); //!< World space
	Vector3 mTotalTorque = math::dvec3Zero(); //!< World space

	std::vector<AppliedForce> mCurrentForces; //!< For visualization purposes. Used to populate mForces in base class.
};

} // namespace sim
} // namespace skybolt