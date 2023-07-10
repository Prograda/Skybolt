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


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/CameraController/CameraControllerSelector.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <map>

namespace skybolt {
namespace sim {

class CameraControllerComponent : public CameraControllerSelector, public Component
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION(Component)
public:
	CameraControllerComponent(const ControllersMap& controllers);
	void updatePostDynamicsSubstep(TimeReal dtSubstep) override;
	void updateAttachments(TimeReal dt, TimeReal dtWallClock) override;
};

SKYBOLT_REFLECT_EXTERN(CameraControllerComponent)

} // namespace sim
} // namespace skybolt