/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EditorPlugin.h"
#include "Sprocket/Entity/EntityPropertiesModel.h"
#include "Sprocket/Property/DefaultEditorWidgets.h"
#include "Sprocket/Scenario/EntityObjectType.h"
#include "Sprocket/Scenario/ScenarioDescObjectType.h"
#include "Sprocket/Scenario/ScenarioPropertiesModel.h"

#include <SkyboltEngine/EngineRoot.h>

using namespace skybolt;

PropertyModelFactoryMap getPropertyModelFactories(const std::vector<EditorPluginPtr>& plugins, EngineRoot* engineRoot)
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

	for (const EditorPluginPtr& plugin : plugins)
	{
		auto items = plugin->getPropertyModelFactories();
		factories.insert(items.begin(), items.end());
	}
	return factories;
}

PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories(const std::vector<EditorPluginPtr>& plugins)
{
	PropertyEditorWidgetFactoryMap factories = getDefaultEditorWidgetFactoryMap();
	for (const EditorPluginPtr& plugin : plugins)
	{
		auto items = plugin->getPropertyEditorWidgetFactories();
		factories.insert(items.begin(), items.end());
	}
	return factories;
}

EntityVisibilityLayerMap getEntityVisibilityLayers(const std::vector<EditorPluginPtr>& plugins)
{
	EntityVisibilityLayerMap r;
	for (const EditorPluginPtr& plugin : plugins)
	{
		auto types = plugin->getEntityVisibilityLayers();
		r.insert(types.begin(), types.end());
	}
	return r;
}