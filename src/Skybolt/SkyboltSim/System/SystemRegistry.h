/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"
#include <SkyboltCommon/Exception.h>
#include <vector>

namespace skybolt {
namespace sim {

typedef std::vector<SystemPtr> SystemRegistry;
typedef std::shared_ptr<SystemRegistry> SystemRegistryPtr;

//! @returns system of type T, or null if none found
template <typename T>
std::shared_ptr<T> findSystem(const SystemRegistry& registry)
{
	for (const auto& i : registry)
	{
		auto r = std::dynamic_pointer_cast<T>(i);
		if (r)
		{
			return r;
		}
	}
	return nullptr;
}

template <typename T>
std::shared_ptr<T> findRequiredSystem(const SystemRegistry& registry)
{
	auto system = sim::findSystem<T>(registry);
	if (!system)
	{
		throw Exception("Could not find system: " + std::string(typeid(T).name()));
	}
	return system;
}


} // namespace sim
} // namespace skybolt