/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Entity.h"
#include "Component.h"
#include "World.h"
#include "Spatial/Geocentric.h"
#include "Spatial/Orientation.h"
#include "Spatial/Position.h"
#include "Components/Motion.h"
#include "Components/Node.h"

namespace skybolt {
namespace sim {

Entity::Entity(const EntityId& id) :
	mId(id)
{
	assert(mId != nullEntityId());
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

void Entity::setDynamicsEnabled(bool enabled)
{
	if (mDynamicsEnabled != enabled)
	{
		mDynamicsEnabled = enabled;

		for (const ComponentPtr& c : mComponents.getAllItems())
		{
			c->setDynamicsEnabled(enabled);
		}
	}
}

void Entity::setSimTime(SecondsD newTime)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
	{
		c->setSimTime(newTime);
	}
}

void Entity::advanceWallTime(SecondsD newTime, SecondsD dt)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
	{
		c->advanceWallTime(newTime, dt);
	}
}

void Entity::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	for (const ComponentPtr& c : mComponents.getAllItems())
	{
		c->advanceSimTime(newTime, dt);
	}
}

void Entity::update(UpdateStage stage)
{
	assert(mDynamicsEnabled || stage != UpdateStage::DynamicsSubStep);

	for (const ComponentPtr& c : mComponents.getAllItems())
	{
		c->update(stage);
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

std::optional<LatLonAlt> getPositionLla(const Entity& entity)
{
	std::optional<Vector3> position = getPosition(entity);
	if (!position) { return std::nullopt; }

	return toLatLonAlt(GeocentricPosition(*position)).position;
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

std::optional<Vector3> getOrientationRpy(const Entity& entity)
{
	std::optional<Vector3> position = getPosition(entity);
	if (!position) { return std::nullopt; }

	std::optional<Quaternion> orientation = getOrientation(entity);
	if (!orientation) { return std::nullopt; }

	Quaternion ltpNed = toLtpNed(GeocentricOrientation(*orientation), geocentricToLatLon(*position)).orientation;
	return math::eulerFromQuat(ltpNed);
}

std::optional<Vector3> getVelocity(const Entity& entity)
{
	auto* component = entity.getFirstComponent<Motion>().get();
	if (component)
	{
		return component->linearVelocity;
	}
	return std::nullopt;
}

std::optional<Vector3> getVelocityNed(const Entity& entity)
{
	std::optional<Vector3> velocity = getVelocity(entity);
	if (!velocity) { return std::nullopt; }

	std::optional<Vector3> position = getPosition(entity);
	if (!position) { return std::nullopt; }

	Matrix3 ltpOrientation = geocentricToLtpOrientation(*position);

	return glm::transpose(ltpOrientation) * *velocity;
}

std::optional<Vector3> getAngularVelocityGeocentricAxes(const Entity& entity)
{
	auto* component = entity.getFirstComponent<Motion>().get();
	if (component)
	{
		return component->angularVelocity;
	}
	return std::nullopt;
}

std::optional<Vector3> getAngularVelocityBodyAxes(const Entity& entity)
{
	auto orientation = getOrientation(entity);
	if (!orientation) { return std::nullopt; }

	auto angularVelocityGeocentric = getAngularVelocityGeocentricAxes(entity);
	if (!angularVelocityGeocentric) { return std::nullopt; }

	return glm::inverse(*orientation) * *angularVelocityGeocentric;
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

void setPositionLla(Entity& entity, const LatLonAlt& lla)
{
	Vector3 position = toGeocentric(LatLonAltPosition(lla)).position;
	setPosition(entity, position);
}

void setOrientation(Entity& entity, const Quaternion& orientation)
{
	if (Node* node = entity.getFirstComponent<Node>().get())
	{
		node->setOrientation(orientation);
	}
}

void setOrientationRpy(Entity& entity, const Vector3& rpy)
{
	std::optional<Vector3> position = getPosition(entity);
	if (!position) { return; }

	Quaternion orientation = toGeocentric(LtpNedOrientation(math::quatFromEuler(rpy)), geocentricToLatLon(*position)).orientation;
	setOrientation(entity, orientation);
}

void setVelocity(Entity& entity, const Vector3& velocity)
{
	if (auto* component = entity.getFirstComponent<Motion>().get())
	{
		component->linearVelocity = velocity;
	}
}

void setVelocityNed(Entity& entity, const Vector3& velocity)
{
	std::optional<Vector3> position = getPosition(entity);
	if (!position) { return; }

	Matrix3 ltpOrientation = geocentricToLtpOrientation(*position);

	setVelocity(entity, ltpOrientation * velocity);
}

void setAngularVelocityGeocentricAxes(const Entity& entity, const Vector3& angularVelocity)
{
	if (auto* component = entity.getFirstComponent<Motion>().get())
	{
		component->angularVelocity = angularVelocity;
	}
}

void setAngularVelocityBodyAxes(const Entity& entity, const Vector3& angularVelocity)
{
	auto orientation = getOrientation(entity);
	if (!orientation) { return; }

	setAngularVelocityGeocentricAxes(entity, *orientation * angularVelocity);
}

} // namespace skybolt
} // namespace sim