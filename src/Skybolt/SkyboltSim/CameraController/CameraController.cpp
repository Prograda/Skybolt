/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SkyboltSim/CameraController/CameraController.h"
#include "SkyboltSim/CollisionGroupMasks.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/World.h"

using namespace skybolt;
using namespace skybolt::sim;

CameraController::CameraController(Entity* camera)
{
	mNodeComponent = camera->getFirstComponent<Node>().get();
	mCameraComponent = camera->getFirstComponent<CameraComponent>().get();
	assert(mCameraComponent);
}

CameraController::~CameraController()
{
	if (mTarget)
	{
		mTarget->removeListener(this);
	}
}

void CameraController::setTarget(Entity* entity)
{
	if (mTarget != entity)
	{
		if (mTarget)
		{
			mTarget->removeListener(this);
		}

		if (entity)
		{
			entity->addListener(this);
		}
		mTarget = entity;
	}
}

void CameraController::onDestroy(Entity* entity)
{
	assert(entity == mTarget);
	setTarget(nullptr);
}
