/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"

namespace skybolt {
namespace sim {

struct ReactionControlSystemParams
{
	Vector3 torque;
};

struct ReactionControlSystemComponentConfig
{
	ReactionControlSystemParams params;
	Node* node;
	DynamicBodyComponent* body;
	ControlInputVec2Ptr stick;
	ControlInputFloatPtr pedal;
};

class ReactionControlSystemComponent : public Component
{
public:
	ReactionControlSystemComponent(const ReactionControlSystemComponentConfig& config);

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::BeginStateUpdate, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updateState();

private:
	const ReactionControlSystemParams mParams;
	Node* mNode;
	DynamicBodyComponent* mBody;
	ControlInputVec2Ptr mStick;
	ControlInputFloatPtr mPedal;
};

} // namespace sim
} // namespace skybolt