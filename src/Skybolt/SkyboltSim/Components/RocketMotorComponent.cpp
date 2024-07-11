/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RocketMotorComponent.h"
#include "DynamicBodyComponent.h"
#include "Node.h"
#include <assert.h>

namespace skybolt {
namespace sim {

RocketMotorComponent::RocketMotorComponent(const RocketMotorComponentParams& params, Node* node, DynamicBodyComponent* body, const ControlInputFloatPtr& input) :
	mParams(params),
	mNode(node),
	mBody(body),
	mInput(input)
{
	assert(mNode);
	assert(mBody);
	assert(mInput);
}

void RocketMotorComponent::updatePreDynamicsSubstep()
{
	float thrust = mInput->value * mParams.maxThrust;
	mBody->applyCentralForce(mNode->getOrientation() * Vector3(thrust, 0, 0));
}

} // namespace sim
} // namespace skybolt