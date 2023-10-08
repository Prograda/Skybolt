/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"

namespace skybolt {
namespace sim {

class NullCameraController : public CameraController
{
public:
	NullCameraController(Entity* camera) : CameraController(camera) {}
};

SKYBOLT_REFLECT_BEGIN(NullCameraController)
{
	registry.type<NullCameraController>("NullCameraController")
		.superType<NullCameraController>();
}
SKYBOLT_REFLECT_END

} // namespace sim
} // namespace skybolt