/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FreeCameraController.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt::sim {

SKYBOLT_REFLECT(FreeCameraController) {
	registry.type<FreeCameraController>("FreeCameraController")
		.superType<CameraController>()
		.superType<CameraModifierStack>()
		.superType<ExplicitSerialization>()
		.superType<Pitchable>()
		.superType<Yawable>()
		.property("minFovY", &FreeCameraController::minFovY, {{PropertyMetadataNames::units, Units::Radians}})
		.property("maxFovY", &FreeCameraController::maxFovY, { {PropertyMetadataNames::units, Units::Radians}});
}


FreeCameraController::FreeCameraController(Entity* camera, const CameraModifierFactoryRegistryPtr& cameraModifierFactories) :
	CameraController(camera),
	CameraModifierStack(cameraModifierFactories)
{
}

void FreeCameraController::updateTimeStep(const UpdateTimeStepArgs& args)
{
	mYaw += mInput.yawRate * args.wallTimeStep;
	mPitch += mInput.tiltRate * args.wallTimeStep;

	if (mInput.zoomRate != 0)
	{
		setZoom(getZoom() + mInput.zoomRate * args.wallTimeStep);
	}
	mCameraComponent->getState().fovY = std::clamp(mCameraComponent->getState().fovY, float(minFovY), float(maxFovY));
	
	double speed = mInput.modifier1Pressed ? 10000.0 : (mInput.modifier2Pressed ? 100.0 : 1000.0);
	Vector3 vel = Vector3(mInput.forwardSpeed, mInput.rightSpeed, 0.0f) * speed;

	// If the node has been moved by external code since the last update, we want to move the base position by the same amount so that the external camera's movement is not overridden by the free camera controller.
	// This allows the free camera controller to be used in conjunction with external code that moves the camera.
	Vector3 nodePositionDelta = mPreviousNodePosition ? (mNodeComponent->getPosition() - *mPreviousNodePosition) : mNodeComponent->getPosition();
	mBasePosition += nodePositionDelta;

	sim::Matrix3 ltpOrientation = geocentricToLtpOrientation(mBasePosition);
	sim::Quaternion ltpOrientationQuat(ltpOrientation);

	Quaternion orientation = ltpOrientationQuat * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0));
	mBasePosition += orientation * vel * args.wallTimeStep;
	Vector3 finalPosition = mBasePosition;

	// Apply camera modifiers
	{
		CameraModifier::State state = {
			.position = finalPosition,
			.orientation = orientation,
			.cameraState = mCameraComponent->getState()
		};
		applyUpdate(*this, state, args.newSimTime, args.simTimeStep);
		finalPosition = state.position;
		orientation = state.orientation;
		mCameraComponent->getState() = state.cameraState;
	}

	// Apply state to camera
	mNodeComponent->setOrientation(orientation);
	mNodeComponent->setPosition(finalPosition);

	mPreviousNodePosition = finalPosition;
}

void FreeCameraController::setActive(bool active)
{
	CameraController::setActive(active);

	// When the free camera controller is activated, update the controller's state variables to match the current camera state.
	if (active)
	{
		mBasePosition = mNodeComponent->getPosition();
		mPreviousNodePosition = mBasePosition;

		sim::Quaternion ltpOrientationQuat(geocentricToLtpOrientation(mBasePosition));
		Quaternion localOrientation = glm::inverse(ltpOrientationQuat) * mNodeComponent->getOrientation();

		Vector3 rpy = math::eulerFromQuat(localOrientation);
		mPitch = rpy.y;
		mYaw = (std::abs(rpy.x) > math::halfPiD()) ? rpy.z + math::piD() : rpy.z;
	}
}

double FreeCameraController::getZoom() const
{
	// Minimum FOV when zoom is 1, maximum FOV when zoom is 0
	double zoom = (mCameraComponent->getState().fovY - maxFovY) / (minFovY - maxFovY);
	return std::clamp(zoom, 0.0, 1.0);
}

void FreeCameraController::setZoom(double zoom)
{
	zoom = skybolt::math::clamp(zoom, 0.0, 1.0);
	mCameraComponent->getState().fovY = skybolt::math::lerp(maxFovY, minFovY, zoom);
}

nlohmann::json FreeCameraController::toJson(refl::TypeRegistry& typeRegistry) const
{
	nlohmann::json json = writeReflectedObjectProperties(typeRegistry, refl::makeRefInstance(typeRegistry, const_cast<FreeCameraController*>(this)));
	if (nlohmann::json modifierJson = CameraModifierStack::toJson(typeRegistry); !modifierJson.is_null())
	{
		json["cameraModifiers"] = modifierJson;
	}
	return json;
}

void FreeCameraController::fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
{
	refl::Instance instance = refl::makeRefInstance(typeRegistry, this);
	readReflectedObjectProperties(typeRegistry, instance, j);
	ifChildExists(j, "cameraModifiers", [&] (const nlohmann::json& modifiersJson) {
		CameraModifierStack::fromJson(typeRegistry, modifiersJson);
	});
}

} // namespace skybolt::sim