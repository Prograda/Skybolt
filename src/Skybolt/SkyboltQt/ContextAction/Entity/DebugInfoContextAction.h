/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/ContextAction/Entity/EntityContextAction.h"
#include <SkyboltSim/Entity.h>

class DebugInfoContextAction : public ContextAction<skybolt::sim::Entity>
{
public:
	std::string getName() const override { return "Debug Info"; }

	bool handles(const skybolt::sim::Entity& entity) const override;

	void execute(skybolt::sim::Entity& entity) const override;
};
