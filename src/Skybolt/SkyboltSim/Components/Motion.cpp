/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Motion.h"
#include "SkyboltSim/Entity.h"

namespace skybolt::sim {

SKYBOLT_REFLECT(Motion)
{
	rttr::registration::class_<Motion>("Motion")
		.property("linearVelocity", &Motion::linearVelocity)
		.property("angularVelocity", &Motion::angularVelocity);
}

} // namespace skybolt::sim