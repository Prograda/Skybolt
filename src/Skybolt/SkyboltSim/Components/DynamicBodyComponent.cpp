/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DynamicBodyComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(DynamicBodyComponent)
{
	rttr::registration::class_<DynamicBodyComponent>("DynamicBodyComponent")
		.property("mass", &DynamicBodyComponent::getMass, &DynamicBodyComponent::setMass);
}

} // namespace sim
} // namespace skybolt