/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OceanComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(OceanComponent)
{
	registry.type<OceanComponent>("OceanComponent")
		.superType<Component>()
		.property("waveHeight", &OceanComponent::waveHeight);
}
SKYBOLT_REFLECT_END

} // namespace sim
} // namespace skybolt