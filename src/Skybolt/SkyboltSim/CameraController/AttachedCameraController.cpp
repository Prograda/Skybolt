/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "AttachedCameraController.h"
#include "SkyboltSim/Components/AttachmentPointsComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

const float AttachedCameraController::msYawRate = 1.0f;
const float AttachedCameraController::msPitchRate = 1.0f;
const float AttachedCameraController::msZoomRate = 1.0f;

AttachedCameraController::AttachedCameraController(sim::Entity* camera, const Params& params) :
	CameraController(camera),
	mParams(params)
{
	setZoom(0.5f);
}

void AttachedCameraController::update(float dt)
{
	mYaw += msYawRate * mInput.yawRate * dt;
	mPitch += msPitchRate * mInput.tiltRate * dt;
	mZoom += msZoomRate * mInput.zoomRate * dt;
	mZoom = math::clamp(mZoom, 0.0f, 1.0f);
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

	CameraState& state = mCameraComponent->getState();
	state.fovY = math::lerp(mParams.maxFovY, mParams.minFovY, mZoom);
	state.nearClipDistance = 0.5;

	if (mTarget && mAttachmentPoint)
	{
		mNodeComponent->setPosition(calcAttachmentPointPosition(*mTarget, *mAttachmentPoint));
		mNodeComponent->setOrientation(calcAttachmentPointOrientation(*mTarget, *mAttachmentPoint) * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0)));
	}
}

void AttachedCameraController::setTarget(Entity* target)
{
	CameraController::setTarget(target);

	mAttachmentPoint = nullptr;
	if (target)
	{
		auto points = target->getFirstComponent<AttachmentPointsComponent>();
		if (points)
		{
			auto point = findOptional(points->attachmentPoints, mParams.attachmentPointName);
			if (point)
			{
				mAttachmentPoint = *point;
			}
		}
	}
}