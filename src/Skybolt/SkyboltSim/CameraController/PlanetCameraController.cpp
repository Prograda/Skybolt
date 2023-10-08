/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "PlanetCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/PlanetComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float PlanetCameraController::msYawRate = 1.f;
const float PlanetCameraController::msPitchRate = 1.f;
const float PlanetCameraController::msZoomRate = 0.2f;
constexpr float pitchControlSensitivity = 2.f;

SKYBOLT_REFLECT_BEGIN(PlanetCameraController)
{
	registry.type<PlanetCameraController>("PlanetCameraController")
		.superType<CameraController>()
		.superType<LatLonSettable>()
		.superType<Pitchable>()
		.superType<Targetable>()
		.superType<Zoomable>();
}
SKYBOLT_REFLECT_END

PlanetCameraController::PlanetCameraController(sim::Entity* camera, World* world, const Params& params) :
	CameraController(camera),
	Targetable(world),
	mParams(params)
{
	setPitch(skybolt::math::halfPiF());
	mCameraComponent->getState().fovY = mParams.fovY;
}

void PlanetCameraController::update(SecondsD dt)
{
	if (Entity* target = getTarget(); target)
	{
		auto position = getPosition(*target);
		auto orientation = getOrientation(*target);
		auto planet = target->getFirstComponent<PlanetComponent>();
		if (!position || !orientation || !planet)
		{
			return;
		}

		float yawDelta = msYawRate * mInput.yawRate * dt;
		float pitchDelta = msPitchRate * mInput.tiltRate * dt;
		float zoomDelta = (mInput.zoomRate + mInput.forwardSpeed) * dt * msZoomRate;
		mZoom = skybolt::math::clamp(mZoom + zoomDelta, 0.0, 1.0);
		float maxDistance = mParams.maxDistOnRadius * planet->radius;

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
}

} // namespace skybolt::sim