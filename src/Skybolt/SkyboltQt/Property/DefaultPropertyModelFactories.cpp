/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DefaultPropertyModelFactories.h"
#include "SkyboltQt/Scenario/EntityObjectType.h"
#include "SkyboltQt/Entity/EntityPropertiesModel.h"
#include "SkyboltQt/Scenario/ScenarioDescObjectType.h"
#include "SkyboltQt/Scenario/ScenarioPropertiesModel.h"

#include <SkyboltEngine/EngineRoot.h>

using namespace skybolt;

PropertyModelFactoryMap getDefaultPropertyModelFactories(EngineRoot* engineRoot)
{
	PropertyModelFactoryMap factories = {
		{ typeid(EntityObject), [engineRoot] (const ScenarioObject& entityItem) {
			sim::EntityId entityId = dynamic_cast<const EntityObject*>(&entityItem)->data;
			return std::make_shared<EntityPropertiesModel>(engineRoot->typeRegistry.get(), engineRoot->scenario->world.getEntityById(entityId).get());
		}},
		{ typeid(ScenarioDescObject), [engineRoot] (const ScenarioObject& entityItem) {
			return std::make_shared<ScenarioPropertiesModel>(engineRoot->scenario.get());
		}}
	};

	return factories;
}