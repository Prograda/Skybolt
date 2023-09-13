/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <Sprocket/Scenario/ScenarioWorkspace.h>
#include <SkyboltEngine/EngineRootFactory.h>

#include <QTemporaryFile>

using namespace skybolt;

static std::shared_ptr<EngineRoot> createEngineRoot()
{
	nlohmann::json engineSettings;
	return EngineRootFactory::create({}, engineSettings);
}

static void createTestNewScenarioEntities(skybolt::sim::World& world, skybolt::EntityFactory& factory)
{
	world.addEntity(factory.createEntity("Camera", "DefaultCamera"));
}

TEST_CASE("Workspace initialized with new scenario")
{
	auto engineRoot = createEngineRoot();
	ScenarioWorkspace workspace(engineRoot, &createTestNewScenarioEntities);
	CHECK(engineRoot->scenario->world.findObjectByName("DefaultCamera") != nullptr);
}

TEST_CASE("Scenario state reset on new scenario")
{
	auto engineRoot = createEngineRoot();
	engineRoot->scenario->world.addEntity(engineRoot->entityFactory->createEntity("Camera", "TestCamera123"));
	
	ScenarioWorkspace workspace(engineRoot, &createTestNewScenarioEntities);
	workspace.newScenario();
	CHECK(engineRoot->scenario->world.findObjectByName("TestCamera123") == nullptr);
}

TEST_CASE("Workspace contains default entities on new scenario")
{
	auto engineRoot = createEngineRoot();
	ScenarioWorkspace workspace(engineRoot, &createTestNewScenarioEntities);
	workspace.newScenario();
	CHECK(!engineRoot->scenario->world.getEntities().empty());
}

TEST_CASE("Workspace saves and loads scenario")
{
	auto engineRoot = createEngineRoot();

	// Save workspace
	QTemporaryFile file;
	{
		ScenarioWorkspace workspace(engineRoot, &createTestNewScenarioEntities);
		engineRoot->scenario->world.addEntity(engineRoot->entityFactory->createEntity("Camera", "TestCamera123"));

		CHECK(workspace.saveScenario(file) == std::nullopt);
		CHECK(workspace.getScenarioFilename() == file.fileName());
	}
	file.close();

	// Load to a different workspace
	ScenarioWorkspace workspace(engineRoot, &createTestNewScenarioEntities);
	CHECK(workspace.loadScenario(file) == std::nullopt);

	CHECK(workspace.getScenarioFilename() == file.fileName());
	CHECK(engineRoot->scenario->world.findObjectByName("TestCamera123") != nullptr);
}
