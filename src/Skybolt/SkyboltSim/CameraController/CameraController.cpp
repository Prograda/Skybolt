/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SkyboltSim/CameraController/CameraController.h"
#include "SkyboltSim/CollisionGroupMasks.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"

namespace skybolt::sim {

CameraController::CameraController(Entity* camera)
{
	mNodeComponent = camera->getFirstComponentRequired<Node>().get();
	mCameraComponent = camera->getFirstComponentRequired<CameraComponent>().get();
}

CameraController::~CameraController()
{
}

} // namespace skybolt::sim