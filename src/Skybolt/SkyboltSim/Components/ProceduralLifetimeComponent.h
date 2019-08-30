/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Component.h>

namespace skybolt {
namespace sim {

//! The lifetime of an entity with this componenent will be managed by procedurally, e.g by a script.
//! The Entity can't be be saved, loaded or deleted by the user.
class ProceduralLifetimeComponent : public sim::Component
{
};

} // namespace sim
} // namespace skybolt