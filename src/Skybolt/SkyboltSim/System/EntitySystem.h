/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include "System.h"
#include <vector>

namespace skybolt {
namespace sim {

class EntitySystem : public System
{
public:
	EntitySystem(World* world);

	void setSimTime(SecondsD newTime) override;
	void advanceWallTime(SecondsD newTime, SecondsD dt) override;
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;
	void update(UpdateStage stage) override;

private:
	World* mWorld;
};

} // namespace sim
} // namespace skybolt