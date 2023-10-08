/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "Pitchable.h"
#include "Targetable.h"
#include "Yawable.h"
#include "Zoomable.h"

namespace skybolt {
namespace sim {

class AttachedCameraController : public CameraController, public Pitchable, public Targetable, public Yawable, public Zoomable
{
public:
	struct Params
	{
		float minFovY;
		float maxFovY;
		std::string attachmentPointName;
	};

	AttachedCameraController(Entity* camera, World* world, const Params& params);

public: // CameraController interface
	void update(SecondsD dt) override;
	void setInput(const Input& input) override { mInput = input; }

private:
	AttachmentPointPtr findAttachmentPoint(const Entity& entity) const; //!< Can return null

private:
	Params mParams;
	Input mInput = Input::zero();

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
};

SKYBOLT_REFLECT_EXTERN(AttachedCameraController)

} // namespace sim
} // namespace skybolt