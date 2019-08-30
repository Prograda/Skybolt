/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimpleNdm.h"

using namespace skybolt;

void SimpleNdm::setNodeDef(const NodeDefPtr& nodeDef)
{
	if (def)
	{
		disconnect(def.get(), &NodeDef::portAdded, this, &SimpleNdm::emitPortAdded);
		disconnect(def.get(), &NodeDef::portMoved, this, &SimpleNdm::emitPortMoved);
		disconnect(def.get(), &NodeDef::portRemoved, this, &SimpleNdm::emitPortRemoved);
	}
	def = nodeDef;
	connect(nodeDef.get(), &NodeDef::portAdded, this, &SimpleNdm::emitPortAdded);
	connect(nodeDef.get(), &NodeDef::portMoved, this, &SimpleNdm::emitPortMoved);
	connect(nodeDef.get(), &NodeDef::portRemoved, this, &SimpleNdm::emitPortRemoved);

	updateParams();
}

void SimpleNdm::updateParams()
{
	// TODO: need to account for param port index changing
	inputParams.resize(def->inputs.size());
	guiParams.resize(def->inputs.size());
	outputParams.resize(def->outputs.size());

	for (int i = 0; i < guiParams.size(); ++i)
	{
		std::shared_ptr<QtNodes::NodeData> node;
		const QString& name = def->inputs[i].name;

		if (guiParams[i] == nullptr || guiParams[i]->type().id != def->inputs[i].id)
		{
			if (def->inputs[i].id == FilenameNodeData::typeId())
			{
				node.reset(new FilenameNodeData(name, ""));
			}
			else if (def->inputs[i].id == StringNodeData::typeId())
			{
				node.reset(new StringNodeData(name, ""));
			}
			else if (def->inputs[i].id == DoubleNodeData::typeId())
			{
				node.reset(new DoubleNodeData(name, 0));
			}
			else if (def->inputs[i].id == IntNodeData::typeId())
			{
				node.reset(new IntNodeData(name, 0));
			}
			else if (def->inputs[i].id == BoolNodeData::typeId())
			{
				node.reset(new BoolNodeData(name, false));
			}
			else if (def->inputs[i].id == StringVectorNodeData::typeId())
			{
				node.reset(new StringVectorNodeData(name, {}));
			}
			else if (def->inputs[i].id == Vector3NodeData::typeId())
			{
				node.reset(new Vector3NodeData(name, sim::Vector3(0, 0, 0)));
			}
			else if (def->inputs[i].id == AngleUnitNodeData::typeId())
			{
				node.reset(new AngleUnitNodeData(name, AngleUnitRadians));
			}
			guiParams[i] = node;
		}
	}
}

void SimpleNdm::emitPortAdded(QtNodes::PortType portType, QtNodes::PortIndex portIndex)
{
	updateParams();
	emit portAdded(portType, portIndex);
}

void SimpleNdm::emitPortMoved(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex)
{
	updateParams();
	emit portMoved(portType, oldIndex, newIndex);
}

void SimpleNdm::emitPortRemoved(QtNodes::PortType portType, QtNodes::PortIndex portIndex)
{
	updateParams();
	emit portRemoved(portType, portIndex);
}

QJsonObject	SimpleNdm::save() const
{
	QJsonObject modelJson = NodeDataModel::save();

	for (const std::shared_ptr<QtNodes::NodeData>& data : guiParams)
	{
		VariantNodeData* variantNodeData = dynamic_cast<VariantNodeData*>(data.get());
		if (variantNodeData)
		{
			QVariant value = variantNodeData->toVariant();
			if (value.type() != QVariant::Invalid)
			{
				modelJson["_" + data->type().name] = value.toString();
			}
		}
	}

	return modelJson;
}

void SimpleNdm::restore(QJsonObject const &p)
{
	for (const std::shared_ptr<QtNodes::NodeData>& param : guiParams)
	{
		VariantNodeData* variantNodeData = dynamic_cast<VariantNodeData*>(param.get());
		if (variantNodeData)
		{
			QJsonValue v = p["_" + param->type().name];

			if (!v.isUndefined())
			{
				variantNodeData->fromVariant(v.toVariant());
			}
		}
	}
	if (guiParams.size())
	{
		eval();
	}
}

void SimpleNdm::eval()
{
	const NodeDataVector& mergedParams = getMergedParams();

	if (outputParams.empty())
	{
		try
		{
			eval(mergedParams);
			errorMessage.clear();
		}
		catch (const std::exception& e)
		{
			errorMessage = e.what();
		}
	}
	else
	{
		for (int i = 0; i < outputParams.size(); ++i)
		{
			try
			{
				outputParams[i] = eval(mergedParams, i);
				errorMessage.clear();
			}
			catch (const std::exception& e)
			{
				errorMessage = e.what();
			}
			emit dataUpdated(i);
		}
	}
}

void SimpleNdm::setInData(std::shared_ptr<QtNodes::NodeData> data, int index)
{
	inputParams[index] = data;
	emit inputChanged(index);
	eval();
}

SimpleNdm::NodeDataVector SimpleNdm::getMergedParams() const
{
	// Get merged result of GUI and input data.
	// Non-null input data replaces GUI data.
	NodeDataVector mergedParams = guiParams;
	int i = 0;
	for (std::shared_ptr<QtNodes::NodeData>& param : mergedParams)
	{
		std::shared_ptr<QtNodes::NodeData> input = inputParams[i];
		if (input)
		{
			param = input;
		}

		++i;
	}
	return mergedParams;
}
