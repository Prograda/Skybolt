/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltCommon/NumericComparison.h>
#include <catch2/catch.hpp>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

constexpr double epsilon = 1e-14;

TEST_CASE("Get Entity Transform")
{
	auto entity = std::make_shared<Entity>(EntityId({1, 1}));

	CHECK(getTransform(*entity) == std::nullopt);

	auto node = std::make_shared<Node>();
	entity->addComponent(node);

	node->setPosition(Vector3(10,20,30));
	node->setOrientation(glm::angleAxis(math::piD(), Vector3(0, 0, 1)));

	auto transform = getTransform(*entity);
	CHECK(transform.has_value());
	Vector3 transformedPoint = *transform * glm::dvec4(1, 2, 3, 1);
	CHECK(transformedPoint.x == Approx(9).margin(epsilon));
	CHECK(transformedPoint.y == Approx(18).margin(epsilon));
	CHECK(transformedPoint.z == Approx(33).margin(epsilon));
}
