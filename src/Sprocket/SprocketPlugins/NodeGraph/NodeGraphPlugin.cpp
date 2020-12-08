/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "NodeGraphPlugin.h"
#include "CodeEditor.h"
#include "DataModelRegistryFactory.h"
#include "NodePropertiesModel.h"
#include "Functions/FlowFunctionPropertiesModel.h"
#include "Functions/GraphFunction.h"
#include "Functions/PythonFunction.h"
#include "Nodes/SimpleNdm.h"
#include <Sprocket/IconFactory.h>
#include <SkyboltEngine/EngineRoot.h>

#include <nodes/ConnectionStyle>
#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/FlowViewStyle>
#include <nodes/Node>

#include <QJsonArray>
#include <QJsonDocument>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>
#include <memory>

using namespace QtNodes;

class PluginFlowFunctionRegistryListener : public FlowFunctionRegistryListener
{
public:
	PluginFlowFunctionRegistryListener(NodeGraphPlugin* nodeGraphPlugin) :
		mNodeGraphPlugin(nodeGraphPlugin)
	{
		assert(mNodeGraphPlugin);
	}

	void itemAdded(const FlowFunctionPtr& function) override
	{
		IconFactory::Icon iconType = dynamic_cast<PythonFunction*>(function.get()) ? IconFactory::Icon::Code : IconFactory::Icon::NodeGraph;
		QIcon icon = getDefaultIconFactory().createIcon(iconType);
		auto item = std::make_shared<FlowFunctionTreeItem>(icon, QString::fromStdString(function->getName()), function.get());
		mNodeGraphPlugin->mFlowFunctionTreeItemRegistry->add(item);
	}

	void itemAboutToBeRemoved(const FlowFunctionPtr& function) override
	{
		if (const GraphFunction* graphFunction = dynamic_cast<const GraphFunction*>(function.get()))
		{
			if (graphFunction && graphFunction->getScene() == mNodeGraphPlugin->mViewedFlowScene)
			{
				mNodeGraphPlugin->viewFlowScene(nullptr);
			}
		}

		if (mNodeGraphPlugin->mViewedPythonFunction == function.get())
		{
			mNodeGraphPlugin->viewPythonFunction(nullptr);
		}

		auto item = mNodeGraphPlugin->mFlowFunctionTreeItemRegistry->findByName(function->getName());
		mNodeGraphPlugin->mFlowFunctionTreeItemRegistry->remove(item.get());
	}

private:
	NodeGraphPlugin* mNodeGraphPlugin;
};

static void setGlobalNodeStyle()
{
	NodeStyle::setNodeStyle(
R"(
  {
    "NodeStyle": {
    "NormalBoundaryColor": [255, 255, 255],
    "SelectedBoundaryColor": [255, 165, 0],
    "GradientColor0": "gray",
    "GradientColor1": [80, 80, 80],
    "GradientColor2": [64, 64, 64],
    "GradientColor3": [58, 58, 58],
    "ShadowColor": [20, 20, 20],
    "FontColor" : "white",
    "FontColorFaded" : "gray",
    "ConnectionPointColor": [100, 100, 100],
    "FilledConnectionPointColor": [180, 180, 180],
    "ErrorColor": "red",
    "WarningColor": [128, 128, 0],
    "PenWidth": 1.0,
    "HoveredPenWidth": 1.5,
    "ConnectionPointDiameter": 8.0,
    "Opacity": 0.8
    }
  }
  )");

	ConnectionStyle::setConnectionStyle(
R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": [20, 150, 220]
    }
  }
  )");
}

static void setNullScene(QtNodes::FlowView& view)
{
	// FlowView crashes with a nullptr, so use null object pattern
	static FlowScene nullScene;
	view.setScene(&nullScene);
}

static FlowView* createFlowView()
{
	FlowView* view = new FlowView();
	setNullScene(*view);
	return view;
}

std::vector<QtNodes::NodeDataType> loadFlowNodeParameters(const QJsonArray& array)
{
	std::vector<QtNodes::NodeDataType> params;

	for (const QJsonValue& value : array)
	{
		const QJsonObject& object = value.toObject();
		
		QtNodes::NodeDataType type;
		type.name = object["name"].toString();
		type.id = object["typeId"].toString();
		params.push_back(type);
	}

	return params;
}

QJsonArray saveFlowNodeParameters(const std::vector<QtNodes::NodeDataType>& params)
{
	QJsonArray array;
	for (const QtNodes::NodeDataType& type : params)
	{
		QJsonObject object;
		object["name"] = type.name;
		object["typeId"] = type.id;
		array.push_back(object);
	}

	return array;
}

void loadFlowFunctions(FlowFunctionRegistry& registry, const QJsonValue& value, std::shared_ptr<DataModelRegistry> dataModelRegistry)
{
	assert(!value.isUndefined());

	QJsonObject json = value.toObject();
	foreach(const QString& key, json.keys())
	{
		QJsonObject value = json.value(key).toObject();
		NodeDefPtr nodeDef(new NodeDef());
		nodeDef->name = key;
		nodeDef->inputs = loadFlowNodeParameters(value["inputs"].toArray());
		nodeDef->outputs = loadFlowNodeParameters(value["outputs"].toArray());

		if (value.contains("graph"))
		{
			auto flowFunction = std::make_shared<GraphFunction>(nodeDef, dataModelRegistry);
			flowFunction->getScene()->clearScene();
			flowFunction->getScene()->loadFromMemory(QJsonDocument(value["graph"].toObject()).toJson());
			registry.add(flowFunction);
		}
		else if (value.contains("pythonScript"))
		{
			auto flowFunction = std::make_shared<PythonFunction>(nodeDef);
			flowFunction->setCode(value["pythonScript"].toString().toStdString()); // TODO: handle compilation exception
			registry.add(flowFunction);
		}
	}
}

QJsonObject saveFlowFunctions(const FlowFunctionRegistry& registry)
{
	QJsonObject json;
	for (const FlowFunctionPtr& flowFunction : registry.getItems())
	{
		QJsonObject object;
		object["inputs"] = saveFlowNodeParameters(flowFunction->getInputs());
		object["outputs"] = saveFlowNodeParameters(flowFunction->getOutputs());

		if (const GraphFunction* graphFunction = dynamic_cast<const GraphFunction*>(flowFunction.get()))
		{
			object["graph"] = QJsonDocument::fromJson(graphFunction->getScene()->saveToMemory()).object();
		}
		else if (const PythonFunction* pythonFunction = dynamic_cast<const PythonFunction*>(flowFunction.get()))
		{
			object["pythonScript"] = QString::fromStdString(pythonFunction->getCode());
		}

		json[QString::fromStdString(flowFunction->getName())] = object;
	}
	return json;
}

typedef TreeItemT<QtNodes::FlowScene*> FlowSceneTreeItem;

NodeGraphPlugin::NodeGraphPlugin(const EditorPluginConfig& config) :
	mUiController(config.uiController),
	mFlowFunctionRegistry(new FlowFunctionRegistry()),
	mFlowFunctionTreeItemRegistry(new Registry<TreeItem>())
{
	mFlowFunctionRegistryListener.reset(new PluginFlowFunctionRegistryListener(this));
	mFlowFunctionRegistry->addListener(mFlowFunctionRegistryListener.get());

	mFlowView = createFlowView();
	setGlobalNodeStyle();
	mToolWindows.push_back({ "Node Graph", mFlowView });

	mCodeEditor = new CodeEditor();
	mToolWindows.push_back({ "Code Editor", mCodeEditor });

	{
		skybolt::EngineRoot* root = config.engineRoot;
		mNodeContext.simWorld = root->simWorld.get();
		mNodeContext.namedObjectRegistry = &root->namedObjectRegistry;
		mNodeContext.flowFunctionRegistry = mFlowFunctionRegistry.get();
		mNodeContext.dataSeriesRegistry = config.dataSeriesRegistry;
		mNodeContext.timeSource = &root->scenario.timeSource;
		mNodeContext.fileLocator = config.fileLocator;
		mNodeContext.entityFactory = root->entityFactory.get();
		mNodeContext.inputPlatform = config.inputPlatform.get();

		mDataModelRegistry = registerDataModels(&mNodeContext);
	}

	mMainFlowScene = new FlowScene(mDataModelRegistry);

	QIcon nodeGraphIcon = getDefaultIconFactory().createIcon(IconFactory::Icon::NodeGraph);
	mFlowFunctionTreeItemRegistry->add(std::make_shared<FlowSceneTreeItem>(nodeGraphIcon, "Main", mMainFlowScene));

	{
		TreeItemType type(typeid(FlowFunctionTreeItem));
		type.name = "Graph Function";
		type.itemCreator = [this](const std::string& name, const std::string& subType) {
			NodeDefPtr nodeDef(new NodeDef);
			nodeDef->name = QString::fromStdString(name);

			auto function = std::make_shared<GraphFunction>(nodeDef, mDataModelRegistry);
			mFlowFunctionRegistry->add(function);
		};
		type.itemDeleter = [this](TreeItem* item) {
			mFlowFunctionRegistry->remove(static_cast<FlowFunctionTreeItem*>(item)->data);
		};
		type.itemRegistry = mFlowFunctionTreeItemRegistry;

		mTreeItemTypes.push_back(type);
	}
}

NodeGraphPlugin::~NodeGraphPlugin()
{
	clearProject();

	delete mFlowView;
	delete mCodeEditor;
}

void NodeGraphPlugin::clearProject()
{
	// Clear scene before rendering and simulation systems shut down so that we don't have dangling references
	mMainFlowScene->clearScene();		
	mFlowFunctionRegistry->clear();
}

void NodeGraphPlugin::loadProject(const QJsonObject& json)
{
	QJsonValue value = json["flowFunctions"];
	if (!value.isUndefined())
	{
		loadFlowFunctions(*mFlowFunctionRegistry, value, mDataModelRegistry);
	}

	value = json["mainFlow"];
	if (!value.isUndefined())
	{
		mMainFlowScene->loadFromMemory(QJsonDocument(value.toObject()).toJson());
	}
}

void NodeGraphPlugin::saveProject(QJsonObject& json)
{
	json["mainFlow"] = QJsonDocument::fromJson(mMainFlowScene->saveToMemory()).object();
	json["flowFunctions"] = saveFlowFunctions(*mFlowFunctionRegistry);
}

std::vector<TreeItemType> NodeGraphPlugin::getTreeItemTypes()
{
	return mTreeItemTypes;
}

std::vector<EditorPlugin::ToolWindow> NodeGraphPlugin::getToolWindows()
{
	return mToolWindows;
}

void NodeGraphPlugin::explorerSelectionChanged(const TreeItem& item)
{
	if (auto flowSceneItem = dynamic_cast<const FlowSceneTreeItem*>(&item))
	{
		viewFlowScene(flowSceneItem->data);
	}
	else if (auto flowFunctionItem = dynamic_cast<const FlowFunctionTreeItem*>(&item))
	{
		if (GraphFunction* graphFunction = dynamic_cast<GraphFunction*>(flowFunctionItem->data))
		{
			viewFlowScene(graphFunction->getScene());
		}
		else if (PythonFunction* pythonFunction = dynamic_cast<PythonFunction*>(flowFunctionItem->data))
		{
			viewPythonFunction(pythonFunction);
		}

		mUiController->propertiesModelSetter(std::make_shared<FlowFunctionPropertiesModel>(flowFunctionItem->data));
	}
}

QRectF getBounds(const FlowScene& scene)
{
	QRectF rect;
	for (const auto& item : scene.nodes())
	{
		rect = rect.united(QRectF(scene.getNodePosition(*item.second), scene.getNodeSize(*item.second)));
	}
	return rect;
}

void NodeGraphPlugin::viewFlowScene(FlowScene* scene)
{
	if (mViewedFlowScene != scene)
	{
		if (!scene)
		{
			setNullScene(*mFlowView);
		}
		else
		{
			mFlowView->setScene(scene);
		}

		mViewedFlowScene = scene;
		flowSceneSelectionChanged();

		QObject::disconnect(mFlowSceneSelectionChangedConnection);
		if (scene)
		{
			mFlowSceneSelectionChangedConnection = QObject::connect(scene, &FlowScene::selectionChanged, [this]() {flowSceneSelectionChanged(); });
		}
	}

	// For some reason we need to call fitInView as well as setSceneRect. TODO: investigate.
	if (scene)
	{
		mFlowView->fitInView(getBounds(*scene), Qt::KeepAspectRatio);
		mFlowView->setSceneRect(getBounds(*scene));
		mUiController->toolWindowRaiser(mFlowView);
	}
}

void NodeGraphPlugin::flowSceneSelectionChanged()
{
	std::vector<QtNodes::Node*> nodes;
	if (mViewedFlowScene)
	{
		nodes = mViewedFlowScene->selectedNodes();
	}

	if (nodes.size() == 1)
	{
		SimpleNdm* nodeDataModel = dynamic_cast<SimpleNdm*>(nodes.front()->nodeDataModel());
		mUiController->propertiesModelSetter(std::make_shared<NodePropertiesModel>(nodeDataModel));
	}
	else
	{
		mUiController->propertiesModelSetter(nullptr);
	}
}

void NodeGraphPlugin::viewPythonFunction(PythonFunction* function)
{
	mViewedPythonFunction = function;

	if (function)
	{
		mCodeEditor->setFilename("PythonFunction: " + QString::fromStdString(function->getName()));

		auto compiler = [this, function](const QString& code) {
			try
			{
				function->setCode(code.toStdString());
			}
			catch (const std::exception& e)
			{
				return QString::fromStdString(e.what());
			}
			return QString("");
		};

		mCodeEditor->setCode(QString::fromStdString(function->getCode()), compiler);
		mUiController->toolWindowRaiser(mCodeEditor);
	}
	else
	{
		mCodeEditor->setFilename("");
		mCodeEditor->setCode("", nullptr);
	}
}

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<NodeGraphPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
