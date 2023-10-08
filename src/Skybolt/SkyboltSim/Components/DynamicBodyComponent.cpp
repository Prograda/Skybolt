/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DynamicBodyComponent.h"

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT_BEGIN(DynamicBodyComponent)
{
	registry.type<DynamicBodyComponent>("DynamicBodyComponent")
		.superType<Component>()
		.property("mass", &DynamicBodyComponent::getMass, &DynamicBodyComponent::setMass);
}
SKYBOLT_REFLECT_END

} // namespace sim
} // namespace skybolt