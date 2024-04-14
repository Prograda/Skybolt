/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraControllerSelector.h"
#include "Targetable.h"

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(CameraControllerSelector)
{
	registry.type<CameraControllerSelector>("CameraControllerSelector")
		.property("selectedController", &CameraControllerSelector::getSelectedControllerName, &CameraControllerSelector::selectController)
		.propertyReadOnly("controllers", &CameraControllerSelector::getControllers);
}
SKYBOLT_REFLECT_END

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
		targetable->setTargetId(getTargetId());
	}
}

void CameraControllerSelector::setTargetId(const EntityId& targetId)
{
	for (const auto& item : mControllers)
	{
		if (auto targetable = dynamic_cast<Targetable*>(item.second.get()); targetable)
		{
			targetable->setTargetId(targetId);
		}
	}
}

EntityId CameraControllerSelector::getTargetId() const
{
	if (mSelectedController)
	{
		if (auto targetable = dynamic_cast<Targetable*>(mSelectedController.get()); targetable)
		{
			return targetable->getTargetId();
		}
	}
	return sim::nullEntityId();
}

} // namespace sim
} // namespace skybolt