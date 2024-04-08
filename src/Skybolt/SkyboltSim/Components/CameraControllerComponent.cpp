/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraControllerComponent.h"
#include "SkyboltSim/CameraController/CameraController.h"
#include "SkyboltSim/CameraController/CameraControllerSelector.h"

#include <SkyboltCommon/Json/JsonHelpers.h>

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(CameraControllerComponent)
{
	registry.type<CameraControllerComponent>("CameraControllerComponent")
		.superType<CameraControllerSelector>()
		.superType<Component>()
		.superType<ExplicitSerialization>();
}
SKYBOLT_REFLECT_END

CameraControllerComponent::CameraControllerComponent(const ControllersMap& controllers) :
	CameraControllerSelector(controllers)
{
}

void CameraControllerComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mSimDt += dt;
}

void CameraControllerComponent::advanceWallTime(SecondsD newTime, SecondsD dt)
{
	mWallDt += dt;
}

void CameraControllerComponent::postDynamicsSubStep()
{
	if (getSelectedController())
	{
		getSelectedController()->updatePostDynamicsSubstep(mSimDt);
	}
	mSimDt = 0;
}

void CameraControllerComponent::updateAttachments()
{
	if (getSelectedController())
	{
		getSelectedController()->update(mWallDt);
	}
	mWallDt = 0;
}

nlohmann::json CameraControllerComponent::toJson(refl::TypeRegistry& typeRegistry) const
{
	nlohmann::json json;
	json["selectedController"] = getSelectedControllerName();

	nlohmann::json controllersJson;
	for (const auto& [name, controller] : getControllers())
	{
		controllersJson[name] = writeReflectedObject(typeRegistry, refl::createNonOwningInstance(&typeRegistry, controller.get()));
	}
	json["controllers"] = controllersJson;
	return json;
}

void CameraControllerComponent::fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
{
	selectController(j.at("selectedController"));

	ifChildExists(j, "controllers", [&] (const nlohmann::json& controllersJson) {
		for (const auto& i : getControllers())
		{
			ifChildExists(controllersJson, i.first, [&, controller = i.second] (const nlohmann::json& controllerJson) {
				refl::Instance instance = refl::createNonOwningInstance(&typeRegistry, controller.get());
				readReflectedObject(typeRegistry, instance, controllerJson);
			});
		}
	});
}

} // namespace sim
} // namespace skybolt