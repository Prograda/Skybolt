/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraControllerSelector.h"
#include "Targetable.h"

#include <assert.h>

using namespace skybolt;
using namespace skybolt::sim;

CameraControllerSelector::CameraControllerSelector(const ControllersMap& controllers) :
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

void CameraControllerSelector::addController(const std::string& name, const CameraControllerPtr& controller)
{
	mControllers[name] = controller;
	if (auto targetable = dynamic_cast<Targetable*>(controller.get()); targetable)
	{
		targetable->setTargetId(mTargetId);
	}
}

void CameraControllerSelector::setTargetId(const EntityId& targetId)
{
	if (targetId != getTargetId())
	{
		for (const auto& item : mControllers)
		{
			if (auto targetable = dynamic_cast<Targetable*>(item.second.get()); targetable)
			{
				targetable->setTargetId(targetId);
			}
		}

		targetChanged(targetId);
	}
}
