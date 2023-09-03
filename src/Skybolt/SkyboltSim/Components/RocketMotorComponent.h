/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

struct RocketMotorComponentParams
{
	float maxThrust;
};

class RocketMotorComponent : public Component
{
public:
	RocketMotorComponent(const RocketMotorComponentParams& params, Node* node, DynamicBodyComponent* body, const ControlInputFloatPtr& input);

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PreDynamicsSubStep, updatePreDynamicsSubstep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamicsSubstep();

private:
	const RocketMotorComponentParams mParams;
	Node* mNode;
	DynamicBodyComponent* mBody;
	ControlInputFloatPtr mInput;
};

} // namespace sim
} // namespace skybolt