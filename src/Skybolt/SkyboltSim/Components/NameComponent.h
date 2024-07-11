/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"

#include <string>

namespace skybolt {
namespace sim {

class NameComponent : public sim::Component
{
public:
	NameComponent(const std::string& name)
	: mName(name)
	{
	}

	const std::string& getName() const {return mName;}

private:
	std::string mName;
};

const std::string& getName(const sim::Entity& entity);

} // namespace sim
} // namespace skybolt