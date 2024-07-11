/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraComponent.h"
#include "SkyboltSim/PropertyMetadata.h"
#include <SkyboltCommon/Units.h>

namespace skybolt::sim {

SKYBOLT_REFLECT_BEGIN(CameraComponent)
{
	registry.type<CameraComponent>("CameraComponent")
		.superType<Component>()
		.property("fovY", &CameraComponent::getFovY, &CameraComponent::setFovY, {{PropertyMetadataNames::units, Units::Radians}});
}
SKYBOLT_REFLECT_END

} // namespace skybolt::sim
