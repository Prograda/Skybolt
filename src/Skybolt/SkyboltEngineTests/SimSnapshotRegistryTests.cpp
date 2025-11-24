/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/Scenario/SimSnapshotRegistry.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltSim/EntityId.h>

using namespace skybolt;

static SimSnapshotRegistry createSimSnapshotRegistry(refl::TypeRegistry* typeRegistry, Scenario* scenario)
{
	assert(scenario);

	// Create snapshot registry
	auto entityFactory = [](const std::string& templateName, const std::string& instanceName) {
		sim::EntityId id{ 1,2 };
		return std::make_shared<sim::Entity>(id);
	};

	sim::EntityPtr entity = entityFactory("myTemplate", "myInstance");

	SimSnapshotRegistryConfig config;
	config.entityFactory = entityFactory;
	config.typeRegistry = typeRegistry;
	config.scenario = scenario;

	return SimSnapshotRegistry(config);
}

TEST_CASE("Snapshot found at time")
{
	refl::TypeRegistry typeRegistry;
	Scenario scenario;
	SimSnapshotRegistry registry = createSimSnapshotRegistry(&typeRegistry, &scenario);

	// Create snapshot at time
	scenario.timeSource.setTime(1);
	registry.saveSnapshotAtCurrentTime();

	// Check that no snapshot is found just before the stored snapshot time
	CHECK(registry.findSnapshotAtTime(0.89, /* epsilon */ 0.1) == nullptr);

	// Check that no snapshot is found just after the stored snapshot time
	CHECK(registry.findSnapshotAtTime(1.11, /* epsilon */ 0.1) == nullptr);

	// Check that snapshot at time is found
	CHECK(registry.findSnapshotAtTime(1));
}

TEST_CASE("Loaded snapshot state identical to saved state")
{
	refl::TypeRegistry typeRegistry;
	Scenario scenario;
	SimSnapshotRegistry registry = createSimSnapshotRegistry(&typeRegistry, &scenario);

	// Create snapshot at time
	scenario.timeSource.setTime(1);
	scenario.timeSource.setRange({0, 2});
	registry.saveSnapshotAtCurrentTime();

	// Modify the scenario
	scenario.timeSource.setRange({ 2, 4 });

	// Load snapshot
	auto snapshot = registry.findSnapshotAtTime(1);
	REQUIRE(snapshot);

	registry.loadSnapshot(*snapshot);

	// Check that the scenario was updated to the saved scenario state
	CHECK(scenario.timeSource.getRange() == TimeRange(0, 2));
}
