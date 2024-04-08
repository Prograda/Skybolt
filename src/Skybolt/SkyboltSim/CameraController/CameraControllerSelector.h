/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include <boost/signals2.hpp>

namespace skybolt {
namespace sim {

class CameraControllerSelector
{
public:
	typedef std::map<std::string, CameraControllerPtr> ControllersMap;
	CameraControllerSelector(const ControllersMap& controllers);

	void selectController(const std::string& name);
	
	const std::string& getSelectedControllerName() const
	{
		return mSelectedName;
	}

	CameraController* getSelectedController() const
	{
		return mSelectedController.get();
	}

	const ControllersMap& getControllers() const
	{
		return mControllers;
	}

	void addController(const std::string& name, const CameraControllerPtr& controller);

	template <class T>
	T* getControllerOfType() const
	{
		for (const auto&[name, controller] : mControllers)
		{
			T* result = dynamic_cast<T*>(controller.get());
			if (result)
			{
				return result;
			}
		}
		return nullptr;
	}

	void setTargetId(const EntityId& target);
	EntityId getTargetId() const;

	boost::signals2::signal<void(const std::string&)> controllerSelected;

private:
	ControllersMap mControllers;
	std::string mSelectedName;
	CameraControllerPtr mSelectedController;
};

SKYBOLT_REFLECT_EXTERN(CameraControllerSelector)

} // namespace sim
} // namespace skybolt