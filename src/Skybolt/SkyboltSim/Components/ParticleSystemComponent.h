/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class ParticleSystemComponent : public Component
{
public:
	ParticleSystemComponent(const ParticleSystemPtr& particleSystem);

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) override;

	const ParticleSystemPtr& getParticleSystem() const { return mParticleSystem; }

private:
	ParticleSystemPtr mParticleSystem;
};

} // namespace sim
} // namespace skybolt
