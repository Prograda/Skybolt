/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MapSamplesNdm.h"
#include "Nodes/NodeContext.h"
#include "Functions/FlowFunction.h"
#include "Functions/FlowFunctionRegistry.h"
#include <SkyboltCommon/Exception.h>

MapSamplesNdm::MapSamplesNdm(NodeContext* context) :
	mFlowFunctionRegistry(context->flowFunctionRegistry)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {{ DoubleVectorMapNodeData::typeId(), "series" },{ StringNodeData::typeId(), "function" } };
	def->outputs = {{ PositionVectorNodeData::typeId(), "position array" }};
	setNodeDef(def);

}

std::shared_ptr<QtNodes::NodeData> MapSamplesNdm:: eval(const NodeDataVector& inputs, int outputIndex) const
{
	std::string functionName = getDataOrDefault<StringNodeData>(inputs[1].get());
	const FlowFunctionPtr& function = mFlowFunctionRegistry->findByName(functionName);
	if (function)
	{
		const DoubleVectorMap& input = getDataOrDefault<DoubleVectorMapNodeData>(inputs[0].get());
		
		size_t elementCount = input.empty() ? 0 : std::numeric_limits<size_t>::max();
		for (const auto& entry : input)
		{
			elementCount = std::min(elementCount, entry.second->data.size());
		}

		PositionVector result(elementCount);
		for (size_t i = 0; i < elementCount; ++i)
		{
			DoubleMap sample;
			for (const auto& entry : input)
			{
				sample[entry.first] = entry.second->data[i];
			}
			auto output = function->eval({ std::make_shared<DoubleMapNodeData>("doubleMap", sample) });
			if (output.empty())
			{
				throw skybolt::Exception("Function '" + functionName + "' returned empty result");
			}

			PositionNodeData* node = dynamic_cast<PositionNodeData*>(output.front().get());
			if (node)
			{
				result[i] = node->data;
			}
			else
			{
				throw skybolt::Exception("Function '" + functionName + "' returned unexpected data type: " + output.front()->type().id.toStdString());
			}
		}

		return toNodeData<PositionVectorNodeData>(outputIndex, result);
	}
	else
	{
		throw skybolt::Exception("Could not find flow function '" + functionName + "'");
	}
	return nullptr;
}
