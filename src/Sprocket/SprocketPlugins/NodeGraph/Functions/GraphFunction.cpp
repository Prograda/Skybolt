/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GraphFunction.h"
#include "Nodes/SimpleNdm.h"
#include <SkyboltCommon/Exception.h>

using namespace QtNodes;

//! Extend DataModelRegistry to add nodes from a second registry
class ExtendedDataModelRegistry : public DataModelRegistry
{
public:
	ExtendedDataModelRegistry(const std::shared_ptr<DataModelRegistry>& wrappedRegistry) :
		mWrappedRegistry(wrappedRegistry)
	{
		assert(mWrappedRegistry);
	}

	std::unique_ptr<NodeDataModel>create(QString const &modelName) override
	{
		std::unique_ptr<NodeDataModel> result = DataModelRegistry::create(modelName);
		if (!result)
		{
			return mWrappedRegistry->create(modelName);
		}
		return result;
	}

	RegisteredModelCreatorsMap const &registeredModelCreators() const override
	{
		mRegisteredModelCreatorsMap = DataModelRegistry::registeredModelCreators();
		RegisteredModelCreatorsMap b = mWrappedRegistry->registeredModelCreators();
		mRegisteredModelCreatorsMap.insert(b.begin(), b.end());
		return mRegisteredModelCreatorsMap;
	}

	RegisteredModelsCategoryMap const &registeredModelsCategoryAssociation() const override
	{
		mRegisteredModelsCategoryMap = DataModelRegistry::registeredModelsCategoryAssociation();
		RegisteredModelsCategoryMap b = mWrappedRegistry->registeredModelsCategoryAssociation();
		mRegisteredModelsCategoryMap.insert(b.begin(), b.end());
		return mRegisteredModelsCategoryMap;
	}

	CategoriesSet const &categories() const override
	{
		mCategoriesSet = DataModelRegistry::categories();
		CategoriesSet b = mWrappedRegistry->categories();
		mCategoriesSet.insert(b.begin(), b.end());
		return mCategoriesSet;
	}

	TypeConverter getTypeConverter(NodeDataType const & d1, NodeDataType const & d2) const override
	{
		TypeConverter result = DataModelRegistry::getTypeConverter(d1, d2);
		if (!result)
		{
			return mWrappedRegistry->getTypeConverter(d1, d2);
		}
		return result;
	}

private:
	std::shared_ptr<DataModelRegistry> mWrappedRegistry;
	// Maps are mutable because getter functions must return const&
	mutable RegisteredModelCreatorsMap mRegisteredModelCreatorsMap;
	mutable RegisteredModelsCategoryMap mRegisteredModelsCategoryMap;
	mutable CategoriesSet mCategoriesSet;
};

class FunctionInputsNode : public SimpleNdm
{
public:
	FunctionInputsNode(const NodeDefPtr& def)
	{
		setNodeDef(def);
	}

	static QString Name() { return "FunctionInputs"; }

	void setInputs(const NodeDataVector& inputs)
	{
		for (unsigned int i = 0; i < inputs.size(); ++i)
		{
			outputParams[i] = eval(inputs, i);
			emit dataUpdated(i);
		}
	}

private:
	std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const override
	{
		return inputs[outputIndex];
	}
};

class FunctionOutputsNode : public SimpleNdm
{
public:
	FunctionOutputsNode(const NodeDefPtr& def)
	{
		setNodeDef(def);
	}

	static QString Name() { return "FunctionOutputs"; }

	void eval(const NodeDataVector& inputs) const override
	{
		outputs = inputs;
	}

	mutable NodeDataVector outputs;
};

GraphFunction::GraphFunction(const NodeDefPtr& nodeDef, const std::shared_ptr<QtNodes::DataModelRegistry>& dataModelRegistry) :
	FlowFunction(nodeDef)
{
	mInputsNodeDef.reset(new NodeDef);
	mInputsNodeDef->name = FunctionInputsNode::Name();
	mInputsNodeDef->outputs = nodeDef->inputs;

	mOutputsNodeDef.reset(new NodeDef);
	mOutputsNodeDef->name = FunctionOutputsNode::Name();
	mOutputsNodeDef->inputs = nodeDef->outputs;

	std::shared_ptr<ExtendedDataModelRegistry> registry(new ExtendedDataModelRegistry(dataModelRegistry));
	registry->registerModel<FunctionInputsNode>([this]() { return std::make_unique<FunctionInputsNode>(mInputsNodeDef);});
	registry->registerModel<FunctionOutputsNode>([this]() { return std::make_unique<FunctionOutputsNode>(mOutputsNodeDef); });

	mScene.reset(new FlowScene(registry));
	
	mScene->createNode(std::make_unique<FunctionInputsNode>(mInputsNodeDef));
	auto node = &mScene->createNode(std::make_unique<FunctionOutputsNode>(mOutputsNodeDef));
	node->nodeGraphicsObject().setPos(QPointF(300, 0));
}

template <typename T>
std::vector<T*> findNodeDataModelsOfType(const QtNodes::FlowScene& scene)
{
	std::vector<T*> nodes;
	for (const auto& entry : scene.nodes())
	{
		T* node = dynamic_cast<T*>(entry.second->nodeDataModel());
		if (node)
		{
			nodes.push_back(node);
		}
	}
	return nodes;
}

QtNodes::PortType getOpposite(const QtNodes::PortType& type)
{
	return (type == QtNodes::PortType::In) ? QtNodes::PortType::Out : QtNodes::PortType::In;
}

void GraphFunction::addPort(QtNodes::PortType portType, QtNodes::PortIndex portIndex, QtNodes::NodeDataType& dataType)
{
	FlowFunction::addPort(portType, portIndex, dataType);

	NodeDefPtr def = (portType == QtNodes::PortType::In) ? mInputsNodeDef : mOutputsNodeDef;
	def->getPorts(getOpposite(portType)) = getNodeDef()->getPorts(portType);
	emit def->portAdded(getOpposite(portType), portIndex);
}

void GraphFunction::movePort(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex)
{
	FlowFunction::movePort(portType, oldIndex, newIndex);

	NodeDefPtr def = (portType == QtNodes::PortType::In) ? mInputsNodeDef : mOutputsNodeDef;
	def->getPorts(getOpposite(portType)) = getNodeDef()->getPorts(portType);
	emit def->portMoved(getOpposite(portType), oldIndex, newIndex);
}

void GraphFunction::removePort(QtNodes::PortType portType, QtNodes::PortIndex portIndex)
{
	FlowFunction::removePort(portType, portIndex);

	NodeDefPtr def = (portType == QtNodes::PortType::In) ? mInputsNodeDef : mOutputsNodeDef;
	def->getPorts(getOpposite(portType)) = getNodeDef()->getPorts(portType);
	emit def->portRemoved(getOpposite(portType), portIndex);
}

NodeDataPtrVector GraphFunction::eval(const NodeDataPtrVector& inputs) const
{
	// Set inputs
	std::vector<FunctionInputsNode*> inputModels = findNodeDataModelsOfType<FunctionInputsNode>(*mScene);
	for (FunctionInputsNode* inputModel : inputModels)
	{
		unsigned int inputCount = inputModel->nPorts(PortType::Out);
		if (inputCount != inputs.size())
		{
			throw skybolt::Exception("Incorrect number of input arguemnts. Found " + std::to_string(inputs.size()) + ", expected " + std::to_string(inputCount));
		}

		inputModel->setInputs(inputs);
	}

	// Get outputs
	// TODO: disallow multiple instances of FunctionOutputsNode in the scene
	std::vector<FunctionOutputsNode*> outputModels = findNodeDataModelsOfType<FunctionOutputsNode>(*mScene);
	return outputModels.empty() ? NodeDataPtrVector() : outputModels.front()->outputs;
}
