/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/Property/PropertyEditor.h"
#include "Sprocket/SprocketFwd.h"
#include "Sprocket/Widgets/WorldTreeWidget.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltCommon/File/FileLocator.h>
#include <QString>
#include <QWidget>

struct UiController
{
	std::function<void(QWidget*)> toolWindowRaiser;
	std::function<void(const PropertiesModelPtr&)> propertiesModelSetter;
};

typedef std::shared_ptr<UiController> UiControllerPtr;

struct EditorPluginConfig
{
	UiControllerPtr uiController;
	skybolt::EngineRoot* engineRoot;
	skybolt::file::FileLocator fileLocator;
	skybolt::InputPlatformPtr inputPlatform;
};

class BOOST_SYMBOL_VISIBLE EditorPlugin
{
public:
	static std::string factorySymbolName() { return "createEditorPlugin"; }

	virtual ~EditorPlugin() {}

	virtual void clearProject() {}
	
	virtual void loadProject(const QJsonObject& json) {}
	
	virtual void saveProject(QJsonObject& json) {}
	
	virtual std::vector<TreeItemType> getTreeItemTypes() { return {}; }

	virtual PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories() { return {}; }

	struct ToolWindow
	{
		QString name;
		QWidget* widget;
	};
	
	virtual std::vector<ToolWindow> getToolWindows() { return {}; }
	
	virtual void explorerSelectionChanged(const TreeItem& item) {}
};
