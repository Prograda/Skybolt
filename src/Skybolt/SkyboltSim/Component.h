/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimUpdatable.h"
#include <SkyboltCommon/TypedItemContainer.h>
#include <SkyboltReflection/Reflection.h>

namespace skybolt {
namespace sim {

class Component : public SimUpdatable
{
public:
	virtual ~Component() {}

	// FIXME: @deprecated. Please call Entity::setDynamicsEnabled instead.
	// Ideally we wouldn't have this method here, but it's needed to allow components to respond to a change in entity dynamics enabled state.
	virtual void setDynamicsEnabled(bool enabled) {};

	//! @returns types this component will be registered as in the type system, used by TypedItemContainer
	virtual std::vector<std::type_index> getExposedTypes() const { return { typeid(*this) }; }
};

} // namespace sim

template<>
inline std::vector<std::type_index> getExposedTypes<sim::Component>(const sim::Component& type)
{
	return type.getExposedTypes();
}

} // namespace skybolt