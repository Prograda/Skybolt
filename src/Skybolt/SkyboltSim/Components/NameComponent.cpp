/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NameComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

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