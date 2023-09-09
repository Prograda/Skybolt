/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/SimpleDynamicBodyComponent.h>
#include <SkyboltSim/SimMath.h>
#include <catch2/catch.hpp>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

static void integrateOverTime(SimpleDynamicBodyComponent& body, SecondsD duration, const std::function<void()>& preStepAction)
{
	constexpr SecondsD dt = 0.01f;
	for (SecondsD t = 0; t < duration; t += dt)
	{
		preStepAction();
		body.advanceSimTime(t, dt);
		body.update(UpdateStage::DynamicsSubStep);
	}
}

TEST_CASE("Body accelerates under force")
{
	Node node;
	Motion motion;
	double mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, &motion, mass, momentOfInertia);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	double t = 3;
	double f = 10;
	double a = f / mass;
	double s = 0.5 * a * t * t;

	// Simulate
	integrateOverTime(body, t, [&] {
		body.applyCentralForce(Vector3(f, 0, 0));
	});

	CHECK(node.getPosition().x == Approx(s).epsilon(0.01));
	CHECK(node.getPosition().y == Approx(0).epsilon(1e-8f));
	CHECK(node.getPosition().z == Approx(0).epsilon(1e-8f));
}

TEST_CASE("Body rotates under torque")
{
	Node node;
	Motion motion;
	double mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, &motion, mass, momentOfInertia);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	float t = 3;
	float T = 0.1f;
	double a = T / momentOfInertia.x;
	double theta = 0.5 * a * t * t;

	// Simulate
	integrateOverTime(body, t, [&] {
		body.applyTorque(Vector3(T, 0, 0));
	});

	Vector3 euler = math::eulerFromQuat(node.getOrientation());

	CHECK(euler.x == Approx(theta).epsilon(0.01));
	CHECK(euler.y == Approx(0).epsilon(1e-8f));
	CHECK(euler.z == Approx(0).epsilon(1e-8f));
}

TEST_CASE("Force at distance produces torque")
{
	Node node;
	Motion motion;
	double mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, &motion, mass, momentOfInertia);
	Vector3 centerOfMass(0.2, 0.3, 0.4);
	body.setCenterOfMass(centerOfMass);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	float t = 3;
	float f = 0.1f;
	float offset = 0.5f;
	double a = f * offset / momentOfInertia.x;
	double theta = 0.5 * a * t * t;

	// Simulate
	integrateOverTime(body, t, [&] {
		body.applyForce(node.getOrientation() * Vector3(0, 0, f), node.getOrientation() * (centerOfMass + Vector3(0, offset, 0)));
	});

	Vector3 euler = math::eulerFromQuat(node.getOrientation());

	CHECK(euler.x == Approx(theta).epsilon(0.01));
	CHECK(euler.y == Approx(0).epsilon(1e-8f));
	CHECK(euler.z == Approx(0).epsilon(1e-8f));
}