/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "OrbitCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/Math/FirstOrderLag.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float OrbitCameraController::msYawRate = 1.0f;
const float OrbitCameraController::msPitchRate = 1.0f;
const float OrbitCameraController::msZoomRate = 1.0f;
const float OrbitCameraController::msPlanetAlignTransitionRate = 2.0f;

SKYBOLT_REFLECT_BEGIN(OrbitCameraController)
{
	registry.type<OrbitCameraController>("OrbitCameraController")
		.superType<CameraController>()
		.superType<Pitchable>()
		.superType<EntityTargeter>()
		.superType<Yawable>()
		.superType<Zoomable>();
}
SKYBOLT_REFLECT_END

OrbitCameraController::OrbitCameraController(sim::Entity* camera, sim::World* world, const Params& params) :
	CameraController(camera),
	EntityTargeter(world),
	mParams(params),
	mTargetOffset(0,0,0),
    mFilteredPlanetUp(0,0,0),
	mTargetPosition(0,0,0)
{
	setZoom(0.5f);
}

void OrbitCameraController::resetFiltering()
{
	mFilteredPlanetUp = Vector3(0,0,0);
	mSmoothedTargetOrientation.reset();
}

void OrbitCameraController::setActive(bool active)
{
	CameraController::setActive(active);
	resetFiltering();
}

static Quaternion safeSlerp(const Quaternion& a, const Quaternion& b, double t)
{
	Quaternion sSafe = (glm::dot(a, b) < 0) ? -a : a;
	return glm::slerp(sSafe, b, t);
}

void OrbitCameraController::updatePostDynamicsSubstep(SecondsD dtSubstep)
{
	if (Entity* entity = getTarget(); entity)
	{
		Quaternion orientation = *getOrientation(*entity);
		if (mSmoothedTargetOrientation)
		{
			orientation = safeSlerp(*mSmoothedTargetOrientation, orientation, calcFirstOrderLagInterpolationFactor(dtSubstep, double(mLagTimeConstant)));
		}
		mSmoothedTargetOrientation = orientation;
	}
}

void OrbitCameraController::update(SecondsD dt)
{
	EntityId targetId = getTargetId();
	if (targetId != mPrevTargetId)
	{
		resetFiltering();
		mPrevTargetId = targetId;
	}

	mCameraComponent->getState().fovY = mParams.fovY;

	mYaw += msYawRate * mInput.yawRate * dt;
	mPitch += msPitchRate * mInput.tiltRate * dt;
	mZoom += msZoomRate * mInput.zoomRate * dt;
	mZoom = math::clamp(mZoom, 0.0, 1.0);
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

//#define ALIGN_CAM_TO_PLANET
#ifdef ALIGN_CAM_TO_PLANET
	SimPlanet* planet;
#endif

	CameraState& state = mCameraComponent->getState();

	if (sim::Entity* target = getTarget(); target)
	{
		state.nearClipDistance = 0.5;
		auto optionalPosition = getPosition(*target);
		if (optionalPosition)
		{
			mTargetPosition = *optionalPosition;

			// Calculate camera orientation
#ifdef ALIGN_CAM_TO_PLANET
			SpaceBody* body = obj->upcastToSpaceBody();
			if (body)
			{
				planet = body->getUniverse()->getNearestPlanet(body->getPosition());
				if (planet)
				{
					// Calculate filtered planet up direction
					{
						Vector3 planetPos = planet->getPosition();
						Vector3 planetUp = body->getPosition() - planetPos;
						planetUp.normalize();

						if (mFilteredPlanetUp.isZero())
						{
							mFilteredPlanetUp = planetUp;
						}
						else
						{
							float delta = std::min(1.0f, dt * msPlanetAlignTransitionRate);
							mFilteredPlanetUp += delta * (planetUp - mFilteredPlanetUp);
							mFilteredPlanetUp.normalize();
						}
					}

					Vector3 camForward = quatRotate(mState.orientation, Vector3(0, 0, -1));
					camForward.normalize();

					Vector3 side = mFilteredPlanetUp.cross(camForward);
					side.normalize();
					camForward = side.cross(mFilteredPlanetUp);


					sim::Matrix3 m(-side, -camForward, mFilteredPlanetUp);
					m.getRotation(mState.orientation);
					mState.orientation = mState.orientation * glm::angleAxis(mYawDelta, Vector3(0, 1, 0))
						* glm::angleAxis(mPitch, Vector3(1, 0, 0));
				}
			}
			else
#endif
			{
				if (!mSmoothedTargetOrientation)
				{
					mSmoothedTargetOrientation = *getOrientation(*target);
				}
				mNodeComponent->setOrientation(*mSmoothedTargetOrientation * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0)));
			}
		}
	}

	// Zoom control
	double dist = mParams.maxDist + mZoom * (mParams.minDist - mParams.maxDist);

	// Derive camera position
	mNodeComponent->setPosition(mTargetPosition + mNodeComponent->getOrientation() * (Vector3(-dist, 0, 0) + mTargetOffset));
}

} // namespace skybolt::sim