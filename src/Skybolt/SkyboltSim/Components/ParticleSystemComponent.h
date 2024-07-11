/* Copyright Matthew Reid
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

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::EndStateUpdate, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void advanceSimTime(SecondsD newTime, SecondsD dt) override;
	void updateState();

	const ParticleSystemPtr& getParticleSystem() const { return mParticleSystem; }

private:
	ParticleSystemPtr mParticleSystem;
	SecondsD mSimTimeDt = 0;
};

} // namespace sim
} // namespace skybolt
