/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OceanComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(OceanComponent)
{
	rttr::registration::class_<OceanComponent>("OceanComponent")
		.property("waveHeight", &OceanComponent::waveHeight);
}

} // namespace sim
} // namespace skybolt