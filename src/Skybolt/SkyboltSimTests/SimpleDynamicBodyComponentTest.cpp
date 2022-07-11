/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Components/SimpleDynamicBodyComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/SimMath.h>
#include <catch2/catch.hpp>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

static void integrateOverTime(SimpleDynamicBodyComponent& body, TimeReal duration, const std::function<void()>& preStepAction)
{
	constexpr TimeReal dt = 0.01f;
	for (TimeReal t = 0; t < duration; t += dt)
	{
		preStepAction();
		body.updateDynamicsSubstep(dt);
	}
}

TEST_CASE("Body accelerates under force")
{
	Node node;
	Real mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, mass, momentOfInertia);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	float t = 3;
	float f = 10;
	Real a = f / mass;
	Real s = 0.5f * a * t * t;

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
	Real mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, mass, momentOfInertia);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	float t = 3;
	float T = 0.1f;
	Real a = T / Real(momentOfInertia.x);
	Real theta = 0.5f * a * t * t;

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
	Real mass = 5;
	Vector3 momentOfInertia(2, 3, 4);
	SimpleDynamicBodyComponent body(&node, mass, momentOfInertia);
	Vector3 centerOfMass(0.2, 0.3, 0.4);
	body.setCenterOfMass(centerOfMass);

	// Calculate expected results
	// https://www.calculatorsoup.com/calculators/physics/displacement_v_a_t.php
	float t = 3;
	float f = 0.1f;
	float offset = 0.5f;
	Real a = f * offset / Real(momentOfInertia.x);
	Real theta = 0.5f * a * t * t;

	// Simulate
	integrateOverTime(body, t, [&] {
		body.applyForce(node.getOrientation() * Vector3(0, 0, f), node.getOrientation() * (centerOfMass + Vector3(0, offset, 0)));
	});

	Vector3 euler = math::eulerFromQuat(node.getOrientation());

	CHECK(euler.x == Approx(theta).epsilon(0.01));
	CHECK(euler.y == Approx(0).epsilon(1e-8f));
	CHECK(euler.z == Approx(0).epsilon(1e-8f));
}