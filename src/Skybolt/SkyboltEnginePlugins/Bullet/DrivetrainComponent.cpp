/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DrivetrainComponent.h"
#include "BulletWheelsComponent.h"

namespace skybolt {
namespace sim {

DrivetrainComponent::DrivetrainComponent(const std::shared_ptr<BulletWheelsComponent>& wheels, const ControlInputFloatPtr& throttle, double maxForce) :
	mWheels(wheels),
	mThrottle(throttle),
	mMaxForce(maxForce)
{
	assert(mThrottle);
}

void DrivetrainComponent::updatePreDynamics()
{
	mWheels->setDrivingForce(mThrottle->value * mMaxForce);
}

} // namespace sim
} // namespace skybolt