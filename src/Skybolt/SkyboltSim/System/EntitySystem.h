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

	void updatePreDynamics(const StepArgs& args) override;
	void updatePreDynamicsSubstep(double dtSubstep) override;
	void updateDynamicsSubstep(double dtSubstep) override;
	void updatePostDynamicsSubstep(double dtSubstep) override;
	void updatePostDynamics(const StepArgs& args) override;

private:
	World* mWorld;
	std::vector<EntityPtr> mEntities;
};

} // namespace sim
} // namespace skybolt