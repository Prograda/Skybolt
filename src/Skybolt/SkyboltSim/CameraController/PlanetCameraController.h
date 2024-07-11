/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "LatLonSettable.h"
#include "Pitchable.h"
#include "Targetable.h"
#include "Zoomable.h"

namespace skybolt {
namespace sim {

class PlanetCameraController : public CameraController, public LatLonSettable, public Pitchable, public Targetable, public Zoomable
{
public:
	struct Params
	{
		double maxDistOnRadius; //!< Maximum alowed camera distance from the planet, divided by planet radius
		float fovY;
		float zoomRate;
	};

	PlanetCameraController(Entity* camera, World* world, const Params& params);

public:
	// CameraController interface
	void update(SecondsD dt) override;
	void setInput(const Input& input) override { mInput = input; }

private:
	Params mParams;
	Input mInput = Input::zero();

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
};

SKYBOLT_REFLECT_EXTERN(PlanetCameraController)

} // namespace sim
} // namespace skybolt