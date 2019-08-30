/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DataSeriesLookupNdm.h"
#include <SkyboltCommon/math/InterpolateTableLinear.h>

DataSeriesLookupNdm::DataSeriesLookupNdm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {
		{ DoubleVectorMapNodeData::typeId(), "data series" },
		{ StringNodeData::typeId(), "x name" },
		{ DoubleNodeData::typeId(), "x value" }
	};
	def->outputs = { { DoubleMapNodeData::typeId(), "y" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> DataSeriesLookupNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	DoubleMap result;

	DoubleVectorMapNodeData* data = dynamic_cast<DoubleVectorMapNodeData*>(inputs[0].get());
	if (data)
	{
		std::string nameX = getDataOrDefault<StringNodeData>(inputs[1].get());
		auto it = data->data.find(nameX);
		if (it != data->data.end())
		{
			const DoubleVector& variableX = it->second->data;
			double x = getDataOrDefault<DoubleNodeData>(inputs[2].get());
			for (const auto& entry : data->data)
			{
				const DoubleVector& variableY = entry.second->data;
				auto value = skybolt::math::interpolateTableLinear(variableX, variableY, x, /*extrapolate*/ false);
				result[entry.first] = value ? *value : 0.0;
			}
		}


	}

	return toNodeData<DoubleMapNodeData>(outputIndex, result);
}