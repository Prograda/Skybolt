/* Copyright Matthew Reid
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
	SimpleDynamicBodyComponent(Node* node, Motion* motion, double mass, const Vector3& momentofInertia);

	double getMass() const override  { return mMass; }

	void setMass(double mass) override { mMass = mass; }
	void setCenterOfMass(const Vector3& relPosition) override { mCenterOfMass = relPosition; }

	//! Apply force at center of mass. Force is in world axes.
	void applyCentralForce(const Vector3& force) override;

	//! Force and relPosition are in world axes
	void applyForce(const Vector3& force, const Vector3& relPosition) override;

	//! Apply torque. Torque is in world axes.
	void applyTorque(const Vector3& torque) override;

	virtual void setCollisionsEnabled(bool enabled) override {}

	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(DynamicBodyComponent), typeid(SimpleDynamicBodyComponent)};
	}

public: // SimUpdatable interface
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::DynamicsSubStep, integrateTimeStep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void integrateTimeStep();

private:
	Node* mNode;
	Motion* mMotion;
	double mMass;
	Vector3 mMomentOfInertia;
	Vector3 mCenterOfMass = math::dvec3Zero();

	SecondsD mElapsedDt;

	Vector3 mTotalForce = math::dvec3Zero(); //!< World space
	Vector3 mTotalTorque = math::dvec3Zero(); //!< World space

	std::vector<AppliedForce> mCurrentForces; //!< For visualization purposes. Used to populate mForcesAppliedInLastSubstep in base class.
};

SKYBOLT_REFLECT_EXTERN(SimpleDynamicBodyComponent);

} // namespace sim
} // namespace skybolt