/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/ContextAction/ActionContext.h"
#include <SkyboltSim/Entity.h>

class MoveToPointContextAction : public DefaultContextAction
{
public:
	std::string getName() const override { return "Move To Point"; }

	bool handles(const ActionContext& context) const override;

	void execute(ActionContext& context) const override;
};
