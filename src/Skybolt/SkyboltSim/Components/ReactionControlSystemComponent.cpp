/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ReactionControlSystemComponent.h"
#include "DynamicBodyComponent.h"
#include "Node.h"
#include <assert.h>

namespace skybolt {
namespace sim {

ReactionControlSystemComponent::ReactionControlSystemComponent(const ReactionControlSystemComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mBody(config.body),
	mStick(config.stick),
	mPedal(config.pedal)
{
	assert(mNode);
	assert(mBody);
	assert(mStick);
	assert(mPedal);
}

void ReactionControlSystemComponent::updateState()
{
	Vector3 throttle(mStick->value, mPedal->value);
	Vector3 torque = throttle * mParams.torque;
	mBody->applyTorque(mNode->getOrientation() * torque);
}

} // namespace sim
} // namespace skybolt