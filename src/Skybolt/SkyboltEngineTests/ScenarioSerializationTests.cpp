/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/Components/TemplateNameComponent.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltReflect/Reflection.h>

using namespace skybolt;

static sim::EntityPtr createEntity(const std::string& templateName, const std::string& instanceName)
{
	static std::uint32_t nextEntityId = 1;
	sim::EntityId id{ 1, nextEntityId++ };
	auto entity = std::make_shared<sim::Entity>(id);
	entity->addComponent(std::make_shared<TemplateNameComponent>(templateName));
	entity->addComponent(std::make_shared<sim::NameComponent>(instanceName));

	auto metadata = std::make_shared<ScenarioMetadataComponent>();
	metadata->lifetimePolicy = ScenarioMetadataComponent::LifetimePolicy::Procedural;
	entity->addComponent(metadata);

	return entity;
};

static void setLifetimePolicy(sim::Entity& entity, ScenarioMetadataComponent::LifetimePolicy policy)
{
	if (auto component = entity.getFirstComponent<ScenarioMetadataComponent>(); component)
	{
		component->lifetimePolicy = policy;
	}
}

TEST_CASE("Scenario is identical after serialization and deserialization")
{
	sim::EntityPtr entity = createEntity("myTemplateName", "myInstanceName");

	refl::TypeRegistry typeRegistry;
	Scenario scenario1;
	scenario1.startJulianDate = 5;
	scenario1.timeSource.setTime(3);
	scenario1.timeSource.setRange({ 2, 4 });
	scenario1.world.addEntity(entity);

	nlohmann::json scenarioJson = writeScenario(typeRegistry, scenario1);

	Scenario scenario2;
	readScenario(typeRegistry, scenario2, &createEntity, scenarioJson);

	CHECK(scenario1.startJulianDate == scenario2.startJulianDate);
	CHECK(scenario1.timeSource.getTime() == scenario2.timeSource.getTime());
	CHECK(scenario1.timeSource.getRange() == scenario2.timeSource.getRange());
	REQUIRE(scenario1.world.getEntities().size() == scenario2.world.getEntities().size());
	sim::EntityPtr scenario2Entity = scenario2.world.findObjectByName("myInstanceName");
	REQUIRE(scenario2Entity);
}

TEST_CASE("On deserialization, new entities created, existing entities updated, old entities deleted")
{
	// Create scenario with two entities
	sim::EntityPtr entityForAddition = createEntity("myTemplate", "entityForAddition");
	sim::EntityPtr entityForModification = createEntity("myTemplate", "entityForModification");

	refl::TypeRegistry typeRegistry;
	Scenario scenario;
	scenario.world.addEntity(entityForAddition);
	scenario.world.addEntity(entityForModification);

	// Disable entityA dynamics so we can test that the dynamics state is serialized/deserialized correctly
	entityForAddition->setDynamicsEnabled(false);

	// Enable entityB dynamics which we'll modify later and then revert by loading the saved state
	entityForModification->setDynamicsEnabled(true);

	// Serialize scenario
	nlohmann::json scenarioJson = writeScenario(typeRegistry, scenario);

	// Remove entity from scenario so we can test that it is added again by load
	scenario.world.removeEntity(entityForAddition.get());

	// Modify entity so we can test that its state is reverted by load
	entityForModification->setDynamicsEnabled(false);

	// Add procedural entity to scenario so we can test that it's deleted by load
	sim::EntityPtr entityProcedural = createEntity("myTemplate", "entityProcedural");
	setLifetimePolicy(*entityProcedural, ScenarioMetadataComponent::LifetimePolicy::Procedural);
	scenario.world.addEntity(entityProcedural);

	// Add user-managed entity to scenario so we can test that it persists after load
	sim::EntityPtr entityPersistant = createEntity("myTemplate", "entityPersistant");
	setLifetimePolicy(*entityPersistant, ScenarioMetadataComponent::LifetimePolicy::User);
	scenario.world.addEntity(entityPersistant);

	// Update scenario by deserializing the saved scenario into it
	readScenario(typeRegistry, scenario, &createEntity, scenarioJson);

	// Check that entityA was added with expected state
	entityForAddition = scenario.world.findObjectByName("entityForAddition");
	REQUIRE(entityForAddition);
	CHECK(!entityForAddition->isDynamicsEnabled());

	// Check that entityForModification state was reverted to the saved state
	CHECK(entityForModification->isDynamicsEnabled());

	// Check that entityProcedural was removed
	CHECK(!scenario.world.findObjectByName("entityProcedural"));

	// Check that entityPersistant was not removed
	CHECK(scenario.world.findObjectByName("entityPersistant"));
}