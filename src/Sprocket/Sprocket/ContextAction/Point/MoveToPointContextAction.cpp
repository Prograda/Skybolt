/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MoveToPointContextAction.h"

using namespace skybolt;
using namespace skybolt::sim;

bool MoveToPointContextAction::handles(const ActionContext& context) const
{
	return context.entity && getPosition(*context.entity).has_value() && context.point;
}

void MoveToPointContextAction::execute(ActionContext& context) const
{
	setPosition(*context.entity, *context.point);
}
