/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CreateContextActions.h"
#include "ContextAction/Entity/AboutContextAction.h"
#include "ContextAction/Entity/AttachToParentContextAction.h"
#include "ContextAction/Entity/DebugInfoContextAction.h"
#include "ContextAction/Entity/DetatchFromParentContextAction.h"
#include "ContextAction/Entity/SetOrientationContextAction.h"
#include "ContextAction/Entity/SetPositionContextAction.h"
#include "ContextAction/Point/MoveToPointContextAction.h"

#include <SkyboltEngine/EngineRoot.h>

using namespace skybolt;

std::vector<DefaultContextActionPtr> createContextActions(const EngineRoot& engineRoot)
{
	return {
		adaptToDefaultAction(std::make_shared<AboutContextAction>()),
		adaptToDefaultAction(std::make_shared<AttachToParentContextAction>(&engineRoot.scenario->world)),
		adaptToDefaultAction(std::make_shared<DetatchFromParentContextAction>()),
		adaptToDefaultAction(std::make_shared<SetPositionContextAction>()),
		adaptToDefaultAction(std::make_shared<SetOrientationContextAction>()),
		adaptToDefaultAction(std::make_shared<DebugInfoContextAction>()),
		std::make_shared<MoveToPointContextAction>()
	};
}