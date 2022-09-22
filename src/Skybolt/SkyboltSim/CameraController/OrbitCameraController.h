/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "Pitchable.h"
#include "Yawable.h"
#include "Zoomable.h"

#include <optional>

namespace skybolt {
namespace sim {

class OrbitCameraController : public CameraController, public Pitchable, public Yawable, public Zoomable
{
public:
	struct Params
	{
		Params(Real _minDist, Real _maxDist, Real _fovY, Real _zoomRate = 0.5f) :
			minDist(_minDist), maxDist(_maxDist), fovY(_fovY), zoomRate(_zoomRate) {}

		Real minDist;
		Real maxDist;
		float fovY;
		float zoomRate;
		Vector3 orientationLagTimeConstant = Vector3(0);
	};

	OrbitCameraController(Entity* camera, const Params& params);

	void setTargetOffset(const Vector3& offset) { mTargetOffset = offset; }
	void setLagTimeConstant(float constant) { mLagTimeConstant = constant; }

public:
	// CameraController interface
	void setActive(bool active) override;
	void updatePostDynamicsSubstep(TimeReal dtSubstep) override;
	void update(float dt) override;
	void setInput(const Input& input) override { mInput = input; }
	void setTarget(Entity* target) override;

private:
	void resetFiltering();

	Params mParams;
	Vector3 mTargetOffset;
	Vector3 mTargetPosition;
	std::optional<Quaternion> mSmoothedTargetOrientation;
	Vector3 mFilteredPlanetUp;
	const Entity* mPrevTarget;
	Input mInput = Input::zero();
	float mLagTimeConstant = 0;

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
	static const float msPlanetAlignTransitionRate;
};

} // namespace sim
} // namespace skybolt