/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NameComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSimFwd.h"
#include <vector>

namespace skybolt {
namespace sim {

void NamedObjectRegistry::add(const std::string& name, sim::Entity* entity)
{
	namedObjects[name] = entity;
	CALL_LISTENERS(objectAdded(name, entity));
}

void NamedObjectRegistry::remove(const std::string& name)
{
	auto it = namedObjects.find(name);
	if (it != namedObjects.end())
	{
		sim::Entity* entity = it->second;
		namedObjects.erase(it);
		CALL_LISTENERS(objectRemoved(name, entity));
	}
}

sim::Entity* NamedObjectRegistry::getObjectByName(const std::string& name) const
{
	auto it = namedObjects.find(name);
	return (it != namedObjects.end()) ? it->second : nullptr;
}

const std::string& getName(const sim::Entity& entity)
{
	std::vector<NameComponentPtr> comp = entity.getComponentsOfType<NameComponent>();
	if (!comp.empty())
	{
		return comp.front()->getName();
	}
	static const std::string empty = "";
	return empty;
}

} // namespace sim
} // namespace skybolt