/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FreeCameraController.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"

#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt::sim {

SKYBOLT_REFLECT_BEGIN(FreeCameraController)
{
	registry.type<FreeCameraController>("FreeCameraController")
		.superType<CameraController>()
		.superType<Pitchable>()
		.superType<Yawable>()
		.superType<Zoomable>();
}
SKYBOLT_REFLECT_END


FreeCameraController::FreeCameraController(Entity* camera, const Params& params) :
	CameraController(camera),
	mBaseFov(params.fovY)
{
	mCameraComponent->getState().fovY = params.fovY;
}

void FreeCameraController::update(SecondsD dt)
{
	mYaw += mInput.yawRate * dt;
	mPitch += mInput.tiltRate * dt;
	mZoom += mInput.zoomRate * dt;
	mZoom = skybolt::math::clamp(mZoom, 0.0, 1.0);

	mCameraComponent->getState().fovY = skybolt::math::lerp(mBaseFov, mBaseFov * 0.1f, float(mZoom));
	
	double speed = mInput.modifier1Pressed ? 10000.0 : (mInput.modifier2Pressed ? 100.0 : 1000.0);
	Vector3 vel = Vector3(mInput.forwardSpeed, mInput.rightSpeed, 0.0f) * speed;

	sim::Matrix3 ltpOrientation = geocentricToLtpOrientation(mNodeComponent->getPosition());
	sim::Quaternion ltpOrientationQuat(ltpOrientation);

	Quaternion orientation = ltpOrientationQuat * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0));
	mNodeComponent->setOrientation(orientation);
	Vector3 position = mNodeComponent->getPosition() + orientation * vel * (double)dt;
	mNodeComponent->setPosition(position);
}

} // namespace skybolt::sim