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
	};

	OrbitCameraController(Entity* camera, const Params& params);

public:
	// CameraController interface
	void setActive(bool active) override;
	void update(float dt) override;
	void setInput(const Input& input) override { mInput = input; }

private:
	void resetFiltering();

	Params mParams;
	Vector3 mTargetOffset;
	Vector3 mTargetPosition;
	Quaternion mTargetOrientation;
	Vector3 mFilteredPlanetUp;
	const Entity* mPrevTarget;
	Input mInput = Input::zero();

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
	static const float msTargetTransitionRate;
	static const float msPlanetAlignTransitionRate;
};

} // namespace sim
} // namespace skybolt