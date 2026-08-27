/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltSim/Components/FuselageComponent.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <catch2/catch.hpp>

#include <cmath>

using namespace skybolt;
using namespace skybolt::sim;

class DummyDynamicBodyComponent : public DynamicBodyComponent
{
public:
	void setMass(double mass) override
	{
		mMass = mass;
	}

	double getMass() const override
	{
		return mMass;
	}

	Vector3 getMomentOfInertia() const override
	{
		return mMomentOfInertia;
	}

	void setMomentOfInertia(const Vector3& momentOfInertia) override
	{
		mMomentOfInertia = momentOfInertia;
	}

	Vector3 getCenterOfMass() const override
	{
		return mCenterOfMass;
	}

	void setCenterOfMass(const Vector3& relPosition) override
	{
		mCenterOfMass = relPosition;
	}

	void applyCentralForce(const Vector3& force) override
	{
		lastCentralForce = force;
	}

	void applyForce(const Vector3& force, const Vector3& relPosition) override
	{
		lastForce = force;
		lastForcePosition = relPosition;
	}

	void applyTorque(const Vector3& torque) override
	{
		lastTorque = torque;
	}

	void setCollisionsEnabled(bool enabled) override
	{
		collisionsEnabled = enabled;
	}

	void setLinearDamping(double damping) override
	{
		linearDamping = damping;
	}

	void setAngularDamping(double damping) override
	{
		angularDamping = damping;
	}

	void setCollisionGroupMask(int mask) override {}
	int getCollisionGroupMask() const override { return 0; }

	void setCollisionFilterMask(int mask) override {}
	int getCollisionFilterMask() const override { return 0; }

	double mMass = 1.0;
	Vector3 mMomentOfInertia = Vector3(2.0, 3.0, 4.0);
	Vector3 mCenterOfMass = math::dvec3Zero();

	Vector3 lastCentralForce = math::dvec3Zero();
	Vector3 lastForce = math::dvec3Zero();
	Vector3 lastForcePosition = math::dvec3Zero();
	Vector3 lastTorque = math::dvec3Zero();

	bool collisionsEnabled = true;
	double linearDamping = 0.0;
	double angularDamping = 0.0;
};

static FuselageParams createParams()
{
	FuselageParams params{};
	params.liftSlope = 5.0;
	params.zeroLiftAlpha = 0.1;
	params.stallAlpha = 0.3;
	params.stallLift = 1.2;
	params.liftArea = 10.0;

	params.dragConstant = Vector3(0.02, 0.03, 0.04);

	params.effectiveWingSpan = 8.0;
	params.wingOswaldEfficiencyFactor = 0.8;
	params.aerodynamicDerivativeReferenceSpeed = 100.0;

	return params;
}

static FuselageComponent createComponent(FuselageParams params, Node& node, Motion& motion, DynamicBodyComponent& body)
{
	return FuselageComponent({
		.params = params,
		.node = &node,
		.motion = &motion,
		.body = &body,
		.stickInput = nullptr,
		.stickTrimInput = nullptr,
		.rudderInput = nullptr
	});
}

TEST_CASE("FuselageComponent::calcLiftCoefficent is linear below stall")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const double alpha = 0.2;
	const double expected = params.liftSlope * (alpha - params.zeroLiftAlpha);

	CHECK(component.calcLiftCoefficent(alpha) == Approx(expected));
}

TEST_CASE("FuselageComponent::calcLiftCoefficent returns zero at zero-lift angle")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	CHECK(component.calcLiftCoefficent(params.zeroLiftAlpha) == Approx(0.0));
}

TEST_CASE("FuselageComponent::calcLiftCoefficent clamps to stalled lift beyond positive stall angle")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const double alphaSlightlyAboveStall = params.zeroLiftAlpha + params.stallAlpha + 0.01;

	CHECK(component.calcLiftCoefficent(alphaSlightlyAboveStall) == Approx(params.stallLift));
}

TEST_CASE("FuselageComponent::calcLiftCoefficent clamps to stalled lift beyond negative stall angle")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const double negativeAlphaSlightlyAboveStall = params.zeroLiftAlpha - params.stallAlpha - 0.01;

	CHECK(component.calcLiftCoefficent(negativeAlphaSlightlyAboveStall) == Approx(params.stallLift));
}

TEST_CASE("FuselageComponent::calcParasiteDragScalar uses axis-weighted quadratic drag")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Vector3 velocityLocal(10.0, 20.0, -30.0);
	const double density = 1.1;

	const double expected =
		0.5 * density *
		(velocityLocal.x * velocityLocal.x * params.dragConstant.x
		+ velocityLocal.y * velocityLocal.y * params.dragConstant.y
		+ velocityLocal.z * velocityLocal.z * params.dragConstant.z);

	CHECK(component.calcParasiteDragScalar(velocityLocal, density) == Approx(expected));
}

TEST_CASE("FuselageComponent::calcInducedDragScalar returns zero when effective wing span is non-positive")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	params.effectiveWingSpan = 0.0;
	FuselageComponent component = createComponent(params, node, motion, body);

	CHECK(component.calcInducedDragScalar(Vector3(50.0, 0.0, 0.0), 1.225, 1000.0) == Approx(0.0));
}

TEST_CASE("FuselageComponent::calcInducedDragScalar increases with lift, but decreases with density and velocity for constant lift")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Vector3 velocityLocal(50.0, 0.0, 0.0);
	const double density = 1.225;
	const double liftForce = 2000.0;

	CHECK(component.calcInducedDragScalar(velocityLocal, density, liftForce * 2) > component.calcInducedDragScalar(velocityLocal, density, liftForce));
	CHECK(component.calcInducedDragScalar(velocityLocal, density * 2, liftForce) < component.calcInducedDragScalar(velocityLocal, density, liftForce));
	CHECK(component.calcInducedDragScalar(velocityLocal * 2.0, density, liftForce) < component.calcInducedDragScalar(velocityLocal, density, liftForce));
}

TEST_CASE("FuselageComponent::calcTotalDragForceInWorldAxes returns zero for zero velocity")
{
	Node node;
	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Vector3 drag = component.calcTotalDragForceInWorldAxes(math::dvec3Zero(), 1.225, 1000.0);

	CHECK(drag.x == Approx(0.0));
	CHECK(drag.y == Approx(0.0));
	CHECK(drag.z == Approx(0.0));
}

TEST_CASE("FuselageComponent::calcTotalDragForceInWorldAxes opposes velocity with parasite and induced drag magnitude")
{
	Node node;
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Vector3 velocityLocal(40.0, 0.0, 0.0);
	const double density = 1.225;
	const double liftForce = 500.0;

	const double parasite = component.calcParasiteDragScalar(velocityLocal, density);
	const double induced = component.calcInducedDragScalar(velocityLocal, density, liftForce);
	const Vector3 drag = component.calcTotalDragForceInWorldAxes(velocityLocal, density, liftForce);

	CHECK(drag.x == Approx(-(parasite + induced)));
	CHECK(drag.y == Approx(0.0));
	CHECK(drag.z == Approx(0.0));
}

TEST_CASE("FuselageComponent::calcMomentInBodyAxes scales with speed squared and air density")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	motion.linearVelocity = Vector3(50.0, 0.0, 0.0);

	DummyDynamicBodyComponent body;
	body.mMomentOfInertia = Vector3(2.0, 3.0, 4.0);

	FuselageParams params = createParams();
	params.pitchAccelDueToElevator = 1;
	params.rollAccelDueToAileron = 2;
	params.yawAccelDueToRudder = 3;
	FuselageComponent component = createComponent(params, node, motion, body);

	const FuselageComponent::Controls controls{
		.stickInput = glm::dvec2(0.25, -0.5),
		.rudderInput = 0.4,
		.collectiveInput = 0.0
	};

	const FuselageComponent::CalcMomentArgs args{
		.controls = controls,
		.airDensity = 1.225,
		.angleOfAttack = -0.1,
		.sideSlipAngle = 0.2,
		.angularVelocityInBodyAxes = Vector3(0.3, -0.4, 0.5)
	};

	const Vector3 baseline = component.calcMomentInBodyAxes(args);

	SECTION("moment scales linearly with air density")
	{
		const Vector3 doubledDensity = component.calcMomentInBodyAxes({
			.controls = args.controls,
			.airDensity = args.airDensity * 2.0,
			.angleOfAttack = args.angleOfAttack,
			.sideSlipAngle = args.sideSlipAngle,
			.angularVelocityInBodyAxes = args.angularVelocityInBodyAxes
		});

		CHECK(doubledDensity.x == Approx(baseline.x * 2.0));
		CHECK(doubledDensity.y == Approx(baseline.y * 2.0));
		CHECK(doubledDensity.z == Approx(baseline.z * 2.0));
	}

	SECTION("aerodynamic contributions scale with square of speed ratio below reference speed")
	{
		motion.linearVelocity = Vector3(100.0, 0.0, 0.0);
		const Vector3 referenceSpeedMoment = component.calcMomentInBodyAxes(args);

		CHECK(referenceSpeedMoment.x != Approx(0.0));
		CHECK(referenceSpeedMoment.y != Approx(0.0));
		CHECK(referenceSpeedMoment.z != Approx(0.0));

		CHECK(referenceSpeedMoment.x == Approx(baseline.x * 4.0));
		CHECK(referenceSpeedMoment.y == Approx(baseline.y * 4.0));
		CHECK(referenceSpeedMoment.z == Approx(baseline.z * 4.0));
	}
}

TEST_CASE("FuselageComponent::calcMomentInBodyAxes clamps speed scaling at reference speed")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	motion.linearVelocity = Vector3(200.0, 0.0, 0.0);

	DummyDynamicBodyComponent body;
	body.mMomentOfInertia = Vector3(1.0, 1.0, 1.0);

	FuselageParams params = createParams();
	params.rollAccelDueToSideSlipAngle = 1.0;
	params.yawAccelDueToSideSlipAngle = 2.0;

	FuselageComponent component = createComponent(params, node, motion, body);

	const double sideSlipAngle = 0.25;
	const double expectedRollMoment = std::get<double>(params.rollAccelDueToSideSlipAngle) * std::sin(sideSlipAngle);

	const Vector3 actual = component.calcMomentInBodyAxes({
		.controls = FuselageComponent::Controls{},
		.airDensity = 1.225,
		.angleOfAttack = 0.0,
		.sideSlipAngle = sideSlipAngle,
		.angularVelocityInBodyAxes = math::dvec3Zero()
	});

	CHECK(actual.x == Approx(expectedRollMoment));
	CHECK(actual.y == Approx(0.0));
	CHECK(actual.z == Approx(std::get<double>(params.yawAccelDueToSideSlipAngle) * std::sin(sideSlipAngle)));
}

TEST_CASE("FuselageComponent::calcLiftForceInWorldAxes returns zero when lift coefficient is zero")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Vector3 lift = component.calcLiftForceInWorldAxes({
		.velocityInBodyAxes = Vector3(100.0, 0.0, 0.0),
		.angleOfAttack = params.zeroLiftAlpha,
		.airDensity = 1.225
	});

	CHECK(lift.x == Approx(0.0));
	CHECK(lift.y == Approx(0.0));
	CHECK(lift.z == Approx(0.0));
}

TEST_CASE("FuselageComponent::updatePreDynamicsSubstep caches angle of attack and sideslip from local velocity")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	motion.linearVelocity = Vector3(100.0, -10.0, 20.0);
	motion.angularVelocity = math::dvec3Zero();

	DummyDynamicBodyComponent body;
	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	component.advanceSimTime(SecondsD(0.1), SecondsD(0.1));
	component.updatePreDynamicsSubstep();

	CHECK(component.getAngleOfAttack() == Approx(std::atan2(20.0, 100.0)));
	CHECK(component.getSideSlipAngle() == Approx(std::atan2(10.0, 100.0)));
}

TEST_CASE("FuselageComponent::calcTrimRotationalMomentInBodyAxes calculates moment for trim condition of zero angular velocity")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	motion.linearVelocity = Vector3(100.0, -10.0, 20.0);
	motion.angularVelocity = Vector3(3.0, -4.0, 5.0); // Give the body some angular velocity so we can check that it is ignored in the trim calculation

	DummyDynamicBodyComponent body;
	body.mMomentOfInertia = Vector3(2.0, 3.0, 4.0);

	FuselageParams params = createParams();
	params.rollAccelDueToRollRate = 1;

	FuselageComponent component = createComponent(params, node, motion, body);

	const Trimmable::Controls controls{};

	const Vector3 velocityLocal = glm::inverse(node.getOrientation()) * motion.linearVelocity;
	const double angleOfAttack = std::atan2(velocityLocal.z, velocityLocal.x);
	const double sideSlipAngle = std::atan2(-velocityLocal.y, velocityLocal.x);

	const Vector3 expected = component.calcMomentInBodyAxes({
		.controls = controls,
		.airDensity = 1.225,
		.angleOfAttack = angleOfAttack,
		.sideSlipAngle = sideSlipAngle,
		.angularVelocityInBodyAxes = math::dvec3Zero()
	});

	const Vector3 actual = component.calcTrimRotationalMomentInBodyAxes(controls);

	CHECK(actual.x == Approx(expected.x));
	CHECK(actual.y == Approx(expected.y));
	CHECK(actual.z == Approx(expected.z));
}

TEST_CASE("FuselageComponent::calcTrimNetForceInWorldAxes ignores vertical velocity for level flight trim")
{
	Node node;
	node.setPosition(Vector3(earthRadius(), 0.0, 0.0));
	node.setOrientation(math::dquatIdentity());

	Motion motion;
	motion.linearVelocity = Vector3(100.0, 0.0, 30.0);

	DummyDynamicBodyComponent body;

	FuselageParams params = createParams();
	FuselageComponent component = createComponent(params, node, motion, body);

	const Trimmable::Controls controls{};

	Vector3 upDir = calcLtpUpDirection(node.getPosition()).value_or(math::dvec3Zero());
	Vector3 horizontalVelocityInWorldAxes = motion.linearVelocity - upDir * glm::dot(upDir, motion.linearVelocity);
	Vector3 horizontalVelocityInBodyAxes = glm::inverse(node.getOrientation()) * horizontalVelocityInWorldAxes;

	const double airDensity = FuselageComponent::calcAirDensity(FuselageComponent::calcAltitude(node.getPosition()));;
	const double angleOfAttack = std::atan2(horizontalVelocityInBodyAxes.z, horizontalVelocityInBodyAxes.x);

	const Vector3 expectedLift = component.calcLiftForceInWorldAxes({
		.velocityInBodyAxes = horizontalVelocityInBodyAxes,
		.angleOfAttack = angleOfAttack,
		.airDensity = airDensity
	});

	const Vector3 expectedDrag = component.calcTotalDragForceInWorldAxes(
		horizontalVelocityInBodyAxes,
		airDensity,
		glm::length(expectedLift));

	const Vector3 expected = expectedLift + expectedDrag;
	const Vector3 actual = component.calcTrimNetForceInWorldAxes(controls);

	CHECK(actual.x == Approx(expected.x));
	CHECK(actual.y == Approx(expected.y));
	CHECK(actual.z == Approx(expected.z));
}