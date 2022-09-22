/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "LatLonSettable.h"
#include "Pitchable.h"
#include "Zoomable.h"

namespace skybolt {
namespace sim {

class PlanetCameraController : public CameraController, public Pitchable, public LatLonSettable, public Zoomable
{
public:
	struct Params
	{
		Real maxDistOnRadius; //!< Maximum alowed camera distance from the planet, divided by planet radius
		float fovY;
		float zoomRate;
	};

	PlanetCameraController(sim::Entity* camera, const Params& params);

public:
	// CameraController interface
	void update(float dt) override;
	void setInput(const Input& input) override { mInput = input; }
	void setTarget(Entity* target) override;

private:
	Params mParams;
	Input mInput = Input::zero();

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
};

} // namespace sim
} // namespace skybolt