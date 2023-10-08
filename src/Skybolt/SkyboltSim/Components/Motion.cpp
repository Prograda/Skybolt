/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Motion.h"
#include "SkyboltSim/Entity.h"

namespace skybolt::sim {

SKYBOLT_REFLECT_BEGIN(Motion)
{
	registry.type<Motion>("Motion")
		.superType<Component>()
		.property("linearVelocity", &Motion::linearVelocity)
		.property("angularVelocity", &Motion::angularVelocity);
}
SKYBOLT_REFLECT_END

} // namespace skybolt::sim