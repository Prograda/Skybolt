/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"

namespace skybolt {
namespace sim {

class BulletWheelsComponent;

class DrivetrainComponent : public Component
{
public:
	DrivetrainComponent(const std::shared_ptr<BulletWheelsComponent>& wheels, const ControlInputFloatPtr& throttle, double maxForce);
	~DrivetrainComponent() override {}

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::BeginStateUpdate, updatePreDynamics)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamics();

private:
	std::shared_ptr<BulletWheelsComponent> mWheels;
	ControlInputFloatPtr mThrottle;
	const double mMaxForce;
};

} // namespace sim
} // namespace skybolt