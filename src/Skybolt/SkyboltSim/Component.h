/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include <SkyboltCommon/TypedItemContainer.h>

namespace skybolt {
namespace sim {

class Component
{
public:
	virtual ~Component() {}

	// TODO: these update hooks should be made more generic. Components should register when they want to be updated.
	virtual void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) {};
	virtual void updatePreDynamicsSubstep(TimeReal dtSubstep) {}
	virtual void updatePostDynamicsSubstep(TimeReal dtSubstep) {}
	virtual void updatePostDynamics(TimeReal dt, TimeReal dtWallClock) {};
	virtual void updateAttachments(TimeReal dt, TimeReal dtWallClock) {};

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