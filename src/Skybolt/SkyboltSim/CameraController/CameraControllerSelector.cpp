/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraControllerSelector.h"

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

CameraControllerSelector::CameraControllerSelector(sim::Entity* camera, const ControllersMap& controllers) :
	CameraController(camera),
	mControllers(controllers)
{
	assert(!mControllers.empty());
	selectController(mControllers.begin()->first);
}

void CameraControllerSelector::selectController(const std::string& name)
{
	if (name != mSelectedName)
	{
		if (mSelectedController)
		{
			mSelectedController->setActive(false);
		}
		mSelectedName = name;

		auto i = mControllers.find(name);
		if (i != mControllers.end())
		{
			mSelectedController = i->second;
			mSelectedController->setActive(true);
		}
		else
		{
			mSelectedController = nullptr;
		}

		controllerSelected(name);
	}
}

void CameraControllerSelector::updatePostDynamicsSubstep(float dtSubstep)
{
	if (mSelectedController)
		mSelectedController->updatePostDynamicsSubstep(dtSubstep);
}

void CameraControllerSelector::update(float dt)
{
	if (mSelectedController)
		mSelectedController->update(dt);
}

void CameraControllerSelector::setInput(const Input& input)
{
	if (mSelectedController)
		mSelectedController->setInput(input);
}

Entity* CameraControllerSelector::getTarget() const
{
	return mSelectedController ? mSelectedController->getTarget() : nullptr;
}

void CameraControllerSelector::setTarget(Entity* target)
{
	if (target != getTarget())
	{
		for (const auto& item : mControllers)
		{
			item.second->setTarget(target);
		}

		targetChanged(target);
	}
}
