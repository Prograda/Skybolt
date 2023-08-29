/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "PlanetCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/PlanetComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

using namespace skybolt::sim;
using namespace skybolt;

const float PlanetCameraController::msYawRate = 1.f;
const float PlanetCameraController::msPitchRate = 1.f;
const float PlanetCameraController::msZoomRate = 0.2f;
constexpr float pitchControlSensitivity = 2.f;

PlanetCameraController::PlanetCameraController(sim::Entity* camera, const Params& params) :
	CameraController(camera),
	mParams(params)
{
	setPitch(skybolt::math::halfPiF());
	mCameraComponent->getState().fovY = mParams.fovY;
}

void PlanetCameraController::update(float dt)
{
	if (!mTarget)
	{
		return;
	}

	auto position = getPosition(*mTarget);
	auto orientation = getOrientation(*mTarget);
	auto planet = mTarget->getFirstComponent<PlanetComponent>();
	if (!position || !orientation || !planet)
	{
		return;
	}

	float yawDelta = msYawRate * mInput.yawRate * dt;
	float pitchDelta = msPitchRate * mInput.tiltRate * dt;
	float zoomDelta = (mInput.zoomRate + mInput.forwardSpeed) * dt * msZoomRate;
	mZoom = skybolt::math::clamp(mZoom + zoomDelta, 0.0f, 1.0f);
	float maxDistance = mParams.maxDistOnRadius * (float)planet->radius;

	// Zoom control
	float exponent = log(maxDistance - (float)planet->radius);
	float distFromSurface = exp(exponent * (1 - mZoom));

	// Orientation control
	if (mInput.modifier1Pressed)
	{
		mPitch -= (double)pitchDelta * pitchControlSensitivity;
		mPitch = skybolt::math::clamp<float>((float)mPitch, 0, skybolt::math::halfPiF());
	}
	else
	{
		float fovHeightAtPlanetSurfaceInMeters = 2.0f * std::tan(mParams.fovY * 0.5f) * distFromSurface;
		float planetSurfaceVisibleVerticalArcInRadians = fovHeightAtPlanetSurfaceInMeters / planet->radius;

		mLatLon.lon -= planetSurfaceVisibleVerticalArcInRadians * yawDelta;
		mLatLon.lat -= planetSurfaceVisibleVerticalArcInRadians * pitchDelta;
		mLatLon.lat = skybolt::math::clamp<double>(mLatLon.lat, -skybolt::math::halfPiD(), skybolt::math::halfPiD());
	}

	Quaternion orientationRelPlanet = *orientation * latLonToGeocentricLtpOrientation(mLatLon) * glm::angleAxis(skybolt::math::halfPiD(), Vector3(0, -1, 0));

	mNodeComponent->setOrientation(orientationRelPlanet * glm::angleAxis(skybolt::math::halfPiD() - mPitch, Vector3(0, 1, 0)));

	Vector3 surfacePosition = *position + orientationRelPlanet * Vector3(-planet->radius, 0, 0);

	// Derive camera position
	mNodeComponent->setPosition(surfacePosition + mNodeComponent->getOrientation() * Vector3(-distFromSurface, 0, 0));
}

void PlanetCameraController::setTarget(Entity* target)
{
	CameraController::setTarget(target);
}