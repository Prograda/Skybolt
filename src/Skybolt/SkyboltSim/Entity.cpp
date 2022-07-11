/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Entity.h"
#include "Component.h"
#include "World.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/Node.h"

namespace skybolt {
namespace sim {

Entity::Entity()
{
}

Entity::~Entity()
{
	CALL_LISTENERS(onDestroy(this));
}

void Entity::addComponent(const ComponentPtr& c)
{
	mComponents.addItem(c);
	CALL_LISTENERS(onComponentAdded(this, c.get()));
}

void Entity::removeComponent(const ComponentPtr& c)
{
	CALL_LISTENERS(onComponentRemove(this, c.get()));
	mComponents.removeItem(c);
}

void Entity::updatePreDynamics(TimeReal dt, TimeReal dtWallClock)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
		c->updatePreDynamics(dt, dtWallClock);
}

void Entity::updatePreDynamicsSubstep(TimeReal dtSubstep)
{
	for(const ComponentPtr& c : mComponents.getAllItems())
		c->updatePreDynamicsSubstep(dtSubstep);
}

void Entity::updateDynamicsSubstep(TimeReal dtSubstep)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
		c->updateDynamicsSubstep(dtSubstep);
}

void Entity::updatePostDynamicsSubstep(TimeReal dtSubstep)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
		c->updatePostDynamicsSubstep(dtSubstep);
}

void Entity::updatePostDynamics(TimeReal dt, TimeReal dtWallClock)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
		c->updatePostDynamics(dt, dtWallClock);
}

void Entity::updateAttachments(TimeReal dt, TimeReal dtWallClock)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
		c->updateAttachments(dt, dtWallClock);
}

void Entity::setDynamicsEnabled(bool enabled)
{
	if (mDynamicsEnabled != enabled)
	{
		mDynamicsEnabled = enabled;

		for (const ComponentPtr& c : mComponents.getAllItems())
			c->setDynamicsEnabled(enabled);
	}
}

Positionable* getPositionable(const Entity& entity)
{
	return entity.getFirstComponent<Node>().get();
}

std::optional<Vector3> getPosition(const Entity& entity)
{
	Positionable* positionable = getPositionable(entity);
	if (positionable)
	{
		return positionable->getPosition();
	}
	return std::nullopt;
}

std::optional<Quaternion> getOrientation(const Entity& entity)
{
	Positionable* positionable = getPositionable(entity);
	if (positionable)
	{
		return positionable->getOrientation();
	}
	return std::nullopt;
}

std::optional<Vector3> getVelocity(const Entity& entity)
{
	auto* component = entity.getFirstComponent<DynamicBodyComponent>().get();
	if (component)
	{
		return component->getLinearVelocity();
	}
	return std::nullopt;
}

std::optional<glm::dmat4> getTransform(const sim::Entity& entity)
{
	Positionable* positionable = getPositionable(entity);
	if (positionable)
	{
		auto r = glm::dmat4(glm::dmat3x3(positionable->getOrientation()));
		auto t = positionable->getPosition();
		r[3][0] = t.x;
		r[3][1] = t.y;
		r[3][2] = t.z;
		return r;
	}
	return std::nullopt;
}

void setPosition(Entity& entity, const Vector3& position)
{
	if (Node* node = entity.getFirstComponent<Node>().get())
	{
		node->setPosition(position);
	}
}

void setOrientation(Entity& entity, const Quaternion& orientation)
{
	if (Node* node = entity.getFirstComponent<Node>().get())
	{
		node->setOrientation(orientation);
	}
}

void setVelocity(Entity& entity, const Vector3& velocity)
{
	if (auto* body = entity.getFirstComponent<DynamicBodyComponent>().get())
	{
		body->setLinearVelocity(velocity);
	}
}

} // namespace skybolt
} // namespace sim