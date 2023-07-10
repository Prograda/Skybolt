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
#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(CameraControllerComponent)
{
	rttr::registration::class_<CameraControllerComponent>("CameraControllerComponent")
		.property("selectedController", &CameraControllerComponent::getSelectedControllerName, &CameraControllerComponent::selectController);
}

CameraControllerComponent::CameraControllerComponent(const ControllersMap& controllers) :
	CameraControllerSelector(controllers)
{
}

void CameraControllerComponent::updatePostDynamicsSubstep(TimeReal dtSubstep)
{
	if (getSelectedController())
	{
		getSelectedController()->updatePostDynamicsSubstep(dtSubstep);
	}
}

void CameraControllerComponent::updateAttachments(TimeReal dt, TimeReal dtWallClock)
{
	if (getSelectedController())
	{
		getSelectedController()->update(dtWallClock);
	}
}

} // namespace sim
} // namespace skybolt