/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraComponent.h"
#include <SkyboltCommon/Units.h>

namespace skybolt::sim {

SKYBOLT_REFLECT(CameraComponent)
{
	rttr::registration::class_<CameraComponent>("CameraComponent")
		.property("fovY", &CameraComponent::getFovY, &CameraComponent::setFovY)
		(    
			rttr::metadata(PropertyMetadataType::Units, Units::Radians)
		);
}

} // namespace skybolt::sim
