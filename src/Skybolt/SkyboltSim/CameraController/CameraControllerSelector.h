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

class CameraControllerSelector : public CameraController
{
public:
	typedef std::map<std::string, CameraControllerPtr> ControllersMap;
	CameraControllerSelector(sim::Entity* camera, const ControllersMap& controllers);

	void selectController(const std::string& name);
	
	std::string getSelectedControllerName() const
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

	void addController(const std::string& name, const CameraControllerPtr& controller)
	{
		mControllers[name] = controller;
	}

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

	boost::signals2::signal<void(const std::string&)> controllerSelected;
	boost::signals2::signal<void(Entity*)> targetChanged;

public:
	// CameraController interface
	void updatePostDynamicsSubstep(float dtSubstep);
	void update(float dt) override;
	void setInput(const Input& input) override;
	Entity* getTarget() const override;
	void setTarget(Entity* target) override;

private:
	ControllersMap mControllers;
	std::string mSelectedName;
	CameraControllerPtr mSelectedController;
};

} // namespace sim
} // namespace skybolt