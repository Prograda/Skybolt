/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ParticleSystemComponent.h"
#include "SkyboltSim/Particles/ParticleSystem.h"

namespace skybolt {
namespace sim {

ParticleSystemComponent::ParticleSystemComponent(const ParticleSystemPtr& particleSystem) :
	mParticleSystem(particleSystem)
{
	assert(mParticleSystem);
}

void ParticleSystemComponent::updatePreDynamics(TimeReal dt, TimeReal dtWallClock)
{
	mParticleSystem->update(dt);
}

} // namespace sim
} // namespace skybolt
