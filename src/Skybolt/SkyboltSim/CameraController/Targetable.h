/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/EntityId.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class Targetable;
SKYBOLT_REFLECT_EXTERN(Targetable)

class Targetable
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION();
public:
	Targetable(World* world);
	virtual const EntityId& getTargetId() const { return mTargetId; }
	virtual void setTargetId(const EntityId& target);

protected:
	Entity* getTarget() const;

private:
	SKYBOLT_REFLECTION_REGISTRATION_FRIEND(Targetable)
	const std::string& getTargetName() const; //! @returns empty string if target is invalid
	void setTargetName(const std::string& targetName);

protected:
	World* mWorld;
	EntityId mTargetId = nullEntityId();
};

} // namespace sim
} // namespace skybolt