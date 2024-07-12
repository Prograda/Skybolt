/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Event.h>
#include <SkyboltCommon/SkyboltCommonFwd.h>
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/System/System.h>

#include <optional>

namespace skybolt::sim {

struct CollisionEvent : public Event
{
	EntityId entityA;
	EntityId entityB;
	Vector3 position; //!< Position of impact point
	Vector3 normalB; //!< Direction of entityB's normal force from the collision
};

struct RayIntersectionResult
{
	Vector3 position;
	Vector3 normal;
	double distance;
	EntityId entity;
};

class CollisionSystem : public System
{
public:
	~CollisionSystem() override = default;
	EventEmitterPtr getEventEmitter() const { return mEventEmitter; }

	virtual std::optional<RayIntersectionResult> intersectRay(const Vector3 &position, const Vector3 &direction, double length, int collisionFilterMask) const
	{
		Vector3 end = position + length * direction;
		return intersectRay(position, end, collisionFilterMask);
	}

	virtual std::optional<RayIntersectionResult> intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask) const { return std::nullopt; };

protected:
	EventEmitterPtr mEventEmitter = std::make_shared<EventEmitter>();
};

} // namespace skybolt::sim