/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "Pitchable.h"
#include "Yawable.h"
#include "Zoomable.h"
#include "Targetable.h"

#include <optional>

namespace skybolt {
namespace sim {

class OrbitCameraController : public CameraController, public Pitchable, public Targetable, public Yawable, public Zoomable
{
public:
	struct Params
	{
		Params(double _minDist, double _maxDist, double _fovY, double _zoomRate = 0.5f) :
			minDist(_minDist), maxDist(_maxDist), fovY(_fovY), zoomRate(_zoomRate) {}

		double minDist;
		double maxDist;
		double fovY;
		double zoomRate;
		Vector3 orientationLagTimeConstant = Vector3(0);
	};

	OrbitCameraController(Entity* camera, World* world, const Params& params);

	void setTargetOffset(const Vector3& offset) { mTargetOffset = offset; }
	void setLagTimeConstant(float constant) { mLagTimeConstant = constant; }

public:
	// CameraController interface
	void setActive(bool active) override;
	void updatePostDynamicsSubstep(SecondsD dtSubstep) override;
	void update(SecondsD dt) override;
	void setInput(const Input& input) override { mInput = input; }

private:
	void resetFiltering();

	Params mParams;
	Vector3 mTargetOffset;
	Vector3 mTargetPosition;
	std::optional<Quaternion> mSmoothedTargetOrientation;
	Vector3 mFilteredPlanetUp;
	EntityId mPrevTargetId = nullEntityId();
	Input mInput = Input::zero();
	float mLagTimeConstant = 0;

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
	static const float msPlanetAlignTransitionRate;
};

SKYBOLT_REFLECT_EXTERN(OrbitCameraController)

} // namespace sim
} // namespace skybolt