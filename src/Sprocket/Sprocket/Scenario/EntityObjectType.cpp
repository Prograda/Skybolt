/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityObjectType.h"
#include "ScenarioObjectIntersectionUtil.h"
#include "Sprocket/Icon/SprocketIcons.h"
#include "Sprocket/Viewport/PlanetPointPicker.h"

#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>

using namespace skybolt;

static bool isDeletable(const sim::Entity& entity)
{
	return !entity.getFirstComponent<sim::ProceduralLifetimeComponent>().get();
}

class EntityObjectRegistry : public Registry<ScenarioObject>, public sim::WorldListener
{
public:
	EntityObjectRegistry(sim::World* world) :
		mWorld(world)
	{
		assert(mWorld);
	}

	void entityAdded(const skybolt::sim::EntityPtr& entity) override
	{
		if (!getName(*entity).empty())
		{
			auto object = std::make_shared<EntityObject>(this, mWorld, *entity);
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
};

EntityObject::EntityObject(EntityObjectRegistry* registry, sim::World* world, const sim::Entity& entity) :
	ScenarioObjectT<skybolt::sim::EntityId>(sim::getName(entity), getSprocketIcon(SprocketIcon::Node), entity.getId()),
	mRegistry(registry),
	mWorld(world)
{
	assert(mRegistry);
	assert(mWorld);
}

static sim::Entity* getParentEntity(const sim::Entity& entity)
{
	sim::ParentReferenceComponent* component = entity.getFirstComponent<sim::ParentReferenceComponent>().get();
	sim::Entity* parent = component ? component->getParent() : nullptr;
	assert(parent != &entity);
	return parent;
}

ScenarioObjectPtr EntityObject::getParent() const
{
	if (sim::Entity* entity = mWorld->getEntityById(data); entity)
	{
		sim::Entity* parent = getParentEntity(*entity);
		return parent ? mRegistry->findByName(sim::getName(*parent)) : nullptr;
	}
	return nullptr;
}

std::optional<skybolt::sim::Vector3> EntityObject::getWorldPosition() const
{
	if (sim::Entity* entity = mWorld->getEntityById(data); entity)
	{
		return getPosition(*entity);
	}
	return std::nullopt;
}

void EntityObject::setWorldPosition(const skybolt::sim::Vector3& position)
{
	if (sim::Entity* entity = mWorld->getEntityById(data); entity)
	{
		return setPosition(*entity, position);
	}
}

std::optional<skybolt::sim::Vector3> EntityObject::intersectRay(const sim::Vector3& origin, const sim::Vector3& dir, const glm::dmat4& viewProjTransform) const
{
	if (sim::Entity* entity = mWorld->getEntityById(data); entity)
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

ScenarioObjectTypePtr createEntityObjectType(sim::World* world, EntityFactory* entityFactory)
{
	auto t = std::make_shared<ScenarioObjectType>();
	t->name = "Entity";
	t->category = "Entity";
	t->templateNames = entityFactory->getTemplateNames();
	t->objectCreator = [world, entityFactory] (const std::string& instanceName, const std::string& templateName) {
		sim::EntityPtr entity = entityFactory->createEntity(templateName);
		world->addEntity(entity);
	};
	t->isObjectRemovable = [world] (const ScenarioObject& object) {
		sim::Entity* entity = world->getEntityById(static_cast<const EntityObject*>(&object)->data);
		return entity ? isDeletable(*entity) : false;
	};
	t->objectRemover = [world] (const ScenarioObject& object) {
		if (sim::Entity* entity = world->getEntityById(static_cast<const EntityObject*>(&object)->data); entity)
		{
			world->removeEntity(entity);
		}
	};
	auto entityObjectRegistry = std::make_shared<EntityObjectRegistry>(world);
	world->addListener(entityObjectRegistry.get());
	t->objectRegistry = entityObjectRegistry;

	return t;
}
