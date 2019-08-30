/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <string>

namespace skybolt {

struct TemplateNameComponent : public sim::Component
{
	TemplateNameComponent(const std::string& name) :
		name(name) {}
	std::string name;
};

} // namespace skybolt