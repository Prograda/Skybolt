/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "AttachedCameraController.h"
#include "SkyboltSim/Components/AttachmentPointsComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float AttachedCameraController::msYawRate = 1.0f;
const float AttachedCameraController::msPitchRate = 1.0f;
const float AttachedCameraController::msZoomRate = 1.0f;

SKYBOLT_REFLECT_BEGIN(AttachedCameraController)
{
	registry.type<AttachedCameraController>("AttachedCameraController")
		.superType<CameraController>()
		.superType<Pitchable>()
		.superType<Targetable>()
		.superType<Yawable>()
		.superType<Zoomable>();
}
SKYBOLT_REFLECT_END

AttachedCameraController::AttachedCameraController(Entity* camera, World* world, const Params& params) :
	CameraController(camera),
	Targetable(world),
	mParams(params)
{
	setZoom(0.5f);
}

void AttachedCameraController::update(SecondsD dt)
{
	mYaw += msYawRate * mInput.yawRate * dt;
	mPitch += msPitchRate * mInput.tiltRate * dt;
	mZoom += msZoomRate * mInput.zoomRate * dt;
	mZoom = math::clamp(mZoom, 0.0, 1.0);
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

	CameraState& state = mCameraComponent->getState();
	state.fovY = math::lerp(mParams.maxFovY, mParams.minFovY, float(mZoom));
	state.nearClipDistance = 0.5;

	if (Entity* target = getTarget(); target)
	{
		if (const AttachmentPointPtr& attachmentPoint = findAttachmentPoint(*target))
		{
			mNodeComponent->setPosition(calcAttachmentPointPosition(*target, *attachmentPoint));
			mNodeComponent->setOrientation(calcAttachmentPointOrientation(*target, *attachmentPoint) * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0)));
		}
	}
}

AttachmentPointPtr AttachedCameraController::findAttachmentPoint(const Entity& entity) const
{
	auto points = entity.getFirstComponent<AttachmentPointsComponent>();
	if (points)
	{
		auto point = findOptional(points->attachmentPoints, mParams.attachmentPointName);
		if (point)
		{
			return *point;;
		}
	}
	return nullptr;
}

} // namespace skybolt::sim