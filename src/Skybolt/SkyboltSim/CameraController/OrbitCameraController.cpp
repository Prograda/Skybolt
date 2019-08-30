/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "OrbitCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

using namespace skybolt::sim;
using namespace skybolt;

const float OrbitCameraController::msYawRate = 0.01f;
const float OrbitCameraController::msPitchRate = 0.01f;
const float OrbitCameraController::msZoomRate = 0.001f;
const float OrbitCameraController::msTargetTransitionRate = 4.0f;
const float OrbitCameraController::msPlanetAlignTransitionRate = 2.0f;


OrbitCameraController::OrbitCameraController(sim::Entity* camera, const Params& params) :
	CameraController(camera),
	mParams(params),
	mTargetOffset(0,0,0),
    mFilteredPlanetUp(0,0,0),
	mTargetPosition(0,0,0),
	mTargetOrientation(0,0,0,1),
	mPrevTarget(nullptr)
{
	setZoom(0.5f);
	mCameraComponent->getState().fovY = mParams.fovY;
}

void OrbitCameraController::resetFiltering()
{
	mFilteredPlanetUp = Vector3(0,0,0);
}

void OrbitCameraController::setActive(bool active)
{
	CameraController::setActive(active);
	resetFiltering();
}

void OrbitCameraController::update(float dt)
{
	if (mTarget != mPrevTarget)
	{
		resetFiltering();
		mPrevTarget = mTarget;
	}

	mYaw += msYawRate * mInput.panSpeed * dt;
	mPitch += msPitchRate * mInput.tiltSpeed * dt;
	float wheel = mInput.zoomSpeed * dt;
	mZoom = math::clamp(mZoom + wheel * msZoomRate, 0.0f, 1.0f);
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

//#define ALIGN_CAM_TO_PLANET
#ifdef ALIGN_CAM_TO_PLANET
	SimPlanet* planet;
#endif

	CameraState& state = mCameraComponent->getState();

	if (mTarget)
	{
		state.nearClipDistance = 0.5;
		auto optionalPosition = getPosition(*mTarget);
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
				mNodeComponent->setOrientation(*getOrientation(*mTarget) * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0)));
			}
		}
	}

	// Zoom control
	float dist = mParams.maxDist + mZoom * (mParams.minDist - mParams.maxDist);

	// Derive camera position
	mNodeComponent->setPosition(mTargetPosition + mTargetOffset + mNodeComponent->getOrientation() * Vector3(-dist, 0, 0));
}
