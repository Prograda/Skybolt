/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "NodeGraphFwd.h"
#include "Functions/FlowFunctionRegistry.h"
#include "Nodes/NodeContext.h"

#include <Sprocket/EditorPlugin.h>
#include <Sprocket/SprocketFwd.h>

#include <nodes/DataModelRegistry>

typedef TreeItemT<FlowFunction*> FlowFunctionTreeItem;

namespace QtNodes {
	class FlowView;
}

class NodeGraphPlugin : public EditorPlugin
{
	friend class PluginFlowFunctionRegistryListener;

public:
	NodeGraphPlugin(const EditorPluginConfig& config);
	~NodeGraphPlugin();

	void clearProject() override;
	
	void loadProject(const QJsonObject& json) override;
	
	void saveProject(QJsonObject& json) override;
	
	std::vector<TreeItemType> getTreeItemTypes() override;
	
	std::vector<ToolWindow> getToolWindows() override;

	void explorerSelectionChanged(const TreeItem& item) override;
	
private:
	void viewFlowScene(QtNodes::FlowScene* scene);
	void flowSceneSelectionChanged();
	void viewPythonFunction(PythonFunction* function);
	
private:
	UiControllerPtr mUiController;
	std::shared_ptr<QtNodes::DataModelRegistry> mDataModelRegistry;
	std::unique_ptr<FlowFunctionRegistry> mFlowFunctionRegistry;
	std::unique_ptr<FlowFunctionRegistryListener> mFlowFunctionRegistryListener;
	NodeContext mNodeContext;
	
	QtNodes::FlowScene* mMainFlowScene;
	QtNodes::FlowView* mFlowView;
	class CodeEditor* mCodeEditor;

	QMetaObject::Connection mFlowSceneSelectionChangedConnection;
	QtNodes::FlowScene* mViewedFlowScene = nullptr;
	PythonFunction* mViewedPythonFunction = nullptr;

	std::shared_ptr<Registry<TreeItem>> mFlowFunctionTreeItemRegistry;
	std::vector<TreeItemType> mTreeItemTypes;

	std::vector<ToolWindow> mToolWindows;
};
