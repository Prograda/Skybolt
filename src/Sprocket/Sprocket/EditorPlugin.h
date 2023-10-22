/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/JsonScenarioSerializable.h"
#include "Sprocket/SprocketFwd.h"
#include "Sprocket/Scenario/ScenarioObject.h"
#include "Sprocket/Scenario/ScenarioObjectTypeMap.h"
#include "Sprocket/Property/PropertyEditor.h"
#include "Sprocket/Property/PropertyModelFactoryMap.h"
#include "Sprocket/Widgets/ScenarioTreeWidget.h"
#include <SkyboltCommon/File/FileLocator.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <QString>
#include <QWidget>

#include <boost/dll/import.hpp>

class QMainWindow;

struct UiController
{
	std::function<void(QWidget*)> toolWindowRaiser;
};

using UiControllerPtr = std::shared_ptr<UiController>;

struct EditorPluginConfig
{
	UiControllerPtr uiController;
	skybolt::EngineRoot* engineRoot;
	skybolt::InputPlatformPtr inputPlatform;
	ScenarioSelectionModel* selectionModel;
	ScenarioObjectTypeMapPtr scenarioObjectTypes;
	skybolt::vis::VisRoot* visRoot;
	QMainWindow* mainWindow;
};

using EntityVisibilityLayerMap = std::map<std::string, skybolt::EntityVisibilityPredicateSetter>;

class BOOST_SYMBOL_VISIBLE EditorPlugin : public JsonScenarioSerializable
{
public:
	static std::string factorySymbolName() { return "createEditorPlugin"; }

	virtual ~EditorPlugin() {}

	virtual PropertyModelFactoryMap getPropertyModelFactories() const { return {}; }

	virtual PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories() const { return {}; }

	virtual EntityVisibilityLayerMap getEntityVisibilityLayers() const { return {}; }

	struct ToolWindow
	{
		QString name;
		QWidget* widget;
	};
	
	virtual std::vector<ToolWindow> getToolWindows() { return {}; }
};

PropertyModelFactoryMap getPropertyModelFactories(const std::vector<EditorPluginPtr>& plugins, skybolt::EngineRoot* engineRoot);
PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories(const std::vector<EditorPluginPtr>& plugins);
EntityVisibilityLayerMap getEntityVisibilityLayers(const std::vector<EditorPluginPtr>& plugins);