/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/EntityId.h"
#include "SkyboltSim/SkyboltSimFwd.h"

#include <SkyboltReflection/Reflection.h>

#include <string>
#include <variant>

namespace skybolt {
namespace sim {

class EntityTargeter
{
public:
	EntityTargeter(World* world);
	virtual ~EntityTargeter() = default;

	virtual const EntityId& getTargetId() const;
	virtual void setTargetId(const EntityId& target);

	Entity* getTarget() const;

	const std::string& getTargetName() const;
	void setTargetName(const std::string& targetName);

protected:
	World* mWorld;

	//! Refer to target entity by name so that we can hold a persistant reference to the entity
	//! regardless of whether the entity currently exists.
	std::string mTargetName;
};

SKYBOLT_REFLECT_EXTERN(EntityTargeter)

} // namespace sim
} // namespace skybolt