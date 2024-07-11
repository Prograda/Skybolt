/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/EntityId.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class Targetable
{
public:
	Targetable(World* world);
	virtual ~Targetable() = default;

	virtual const EntityId& getTargetId() const { return mTargetId; }
	virtual void setTargetId(const EntityId& target);

	Entity* getTarget() const;

	// Helper methods used by serialization. FIXME: These should be hidden.
	const std::string& getTargetName() const;
	void setTargetName(const std::string& targetName);

protected:
	World* mWorld;
	EntityId mTargetId = nullEntityId();
};

SKYBOLT_REFLECT_EXTERN(Targetable)

} // namespace sim
} // namespace skybolt