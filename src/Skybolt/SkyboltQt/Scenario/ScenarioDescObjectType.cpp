/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioDescObjectType.h"
#include "SkyboltQt/Icon/SkyboltIcons.h"

#include <SkyboltEngine/Scenario/Scenario.h>

using namespace skybolt;

ScenarioObjectTypePtr createScenarioDescObjectType(skybolt::Scenario* scenario)
{
	auto object = std::make_shared<ScenarioDescObject>("Scenario", getSkyboltIcon(SkyboltIcon::Node), scenario);

	auto t = std::make_shared<ScenarioObjectType>();
	t->name = "ScenarioDesc";
	t->isObjectRemovable = [] (const ScenarioObject& object) {
		return false;
	};
	t->objectRemover = [] (const ScenarioObject& object) {};
	t->objectRegistry = std::make_shared<ScenarioObjectRegistry>();
	t->objectRegistry->add(object);
	return t;
}
