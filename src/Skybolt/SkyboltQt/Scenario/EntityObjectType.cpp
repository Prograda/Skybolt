/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityObjectType.h"
#include "ScenarioObjectIntersectionUtil.h"
#include "SkyboltQt/Icon/SkyboltIcons.h"
#include "SkyboltQt/Viewport/PlanetPointPicker.h"

#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>

using namespace skybolt;

static bool isDeletable(const sim::Entity& entity)
{
	if (auto metadata = entity.getFirstComponent<ScenarioMetadataComponent>(); metadata)
	{
		return metadata->deletable;
	}
	return false;
}

class EntityObjectRegistry : public ObservableRegistry<ScenarioObject>, public sim::WorldListener
{
public:
	EntityObjectRegistry(sim::World* world, EntityObjectFactory entityObjectFactory) :
		mWorld(world),
		mEntityObjectFactory(std::move(entityObjectFactory))
	{
		assert(mWorld);
		assert(mEntityObjectFactory);

		for (const auto& entity : mWorld->getEntities())
		{
			entityAdded(entity);
		}
	}

	void entityAdded(const skybolt::sim::EntityPtr& entity) override
	{
		if (!getName(*entity).empty())
		{
			auto object = mEntityObjectFactory(this, mWorld, *entity);
			add(object);
		}
	}

	void entityRemoved(const skybolt::sim::EntityPtr& entity) override
	{
		if (const std::string& name = getName(*entity); !name.empty())
		{
			EntityObject* object = static_cast<EntityObject*>(findByName(name).get());
			remove(object);
		}
	}

private:
	sim::World* mWorld;
	EntityObjectFactory mEntityObjectFactory;
};

EntityObject::EntityObject(EntityObjectRegistry* registry, sim::World* world, const sim::Entity& entity) :
	ScenarioObjectT<skybolt::sim::EntityId>(sim::getName(entity), getSkyboltIcon(SkyboltIcon::Node), entity.getId()),
	mRegistry(registry),
	mWorld(world)
{
	assert(mRegistry);
	assert(mWorld);
}

const ScenarioObjectPath& EntityObject::getDirectory() const
{
	if (sim::Entity* entity = mWorld->getEntityById(data).get(); entity)
	{
		if (auto component = entity->getFirstComponent<ScenarioMetadataComponent>().get(); component)
		{
			return component->directory;
		}
	}
	return mDirectory;
}

void EntityObject::setDirectory(const ScenarioObjectPath& path)
{
	if (sim::Entity* entity = mWorld->getEntityById(data).get(); entity)
	{
		if (auto component = entity->getFirstComponent<ScenarioMetadataComponent>().get(); component)
		{
			component->directory = path;
		}
	}
	mDirectory = path;
}

std::optional<skybolt::sim::Vector3> EntityObject::getWorldPosition() const
{
	if (sim::Entity* entity = mWorld->getEntityById(data).get(); entity)
	{
		return getPosition(*entity);
	}
	return std::nullopt;
}

void EntityObject::setWorldPosition(const skybolt::sim::Vector3& position)
{
	if (sim::Entity* entity = mWorld->getEntityById(data).get(); entity)
	{
		return setPosition(*entity, position);
	}
}

std::optional<skybolt::sim::Vector3> EntityObject::intersectRay(const sim::Vector3& origin, const sim::Vector3& dir, const glm::dmat4& viewProjTransform) const
{
	if (sim::Entity* entity = mWorld->getEntityById(data).get(); entity)
	{
		if (auto component = entity->getFirstComponent<sim::PlanetComponent>().get(); component)
		{
			// If the entity is a planet, intersect the actual planet surface
			if (auto pickedPosition = pickPointOnPlanet(*entity, *component, origin, dir); pickedPosition)
			{
				return pickedPosition;
			}
		}
		else
		{
			// For other entity types, intersect with a sphere of fixed screenspace size at the object's position
			if (auto entityPositionWorld = getPosition(*entity); entityPositionWorld)
			{
				return intersectRayWithIcon(origin, dir, viewProjTransform, *entityPositionWorld);
			}
		}
	}
	return std::nullopt;
}

EntityObjectFactory createDefaultEntityObjectFactory()
{
	return [] (EntityObjectRegistry* registry, skybolt::sim::World* world, const skybolt::sim::Entity& entity) {
		return std::make_shared<EntityObject>(registry, world, entity);
	};
}

ScenarioObjectTypePtr createEntityObjectType(sim::World* world, EntityFactory* entityFactory, EntityObjectFactory entityObjectFactory)
{
	auto t = std::make_shared<ScenarioObjectType>();
	t->name = "Entity";
	t->templateNames = entityFactory->getTemplateNames();
	t->getScenarioObjectDirectoryForTemplate = [entityFactory] (const std::string& templateName) {
		return entityFactory->getScenarioObjectDirectoryForTemplate(templateName);
	};
	t->objectCreator = [world, entityFactory] (const std::string& instanceName, const std::string& templateName) {
		sim::EntityPtr entity = entityFactory->createEntity(templateName);
		world->addEntity(entity);
	};
	t->isObjectRemovable = [world] (const ScenarioObject& object) {
		sim::Entity* entity = world->getEntityById(static_cast<const EntityObject*>(&object)->data).get();
		return entity ? isDeletable(*entity) : false;
	};
	t->objectRemover = [world] (const ScenarioObject& object) {
		if (sim::Entity* entity = world->getEntityById(static_cast<const EntityObject*>(&object)->data).get(); entity)
		{
			world->removeEntity(entity);
		}
	};
	auto entityObjectRegistry = std::make_shared<EntityObjectRegistry>(world, std::move(entityObjectFactory));
	world->addListener(entityObjectRegistry.get());
	t->objectRegistry = entityObjectRegistry;

	return t;
}
