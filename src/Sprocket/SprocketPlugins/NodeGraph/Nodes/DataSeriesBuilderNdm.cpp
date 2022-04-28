/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DataSeriesBuilderNdm.h"
#include "NodeContext.h"

using namespace skybolt;

DataSeriesBuilderNdm::DataSeriesBuilderNdm(NodeContext* context) :
	mContext(context),
	mPrevTime(0)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { StringVectorNodeData::typeId(), "names" },{ DoubleNodeData::typeId(), "x" },{ DoubleNodeData::typeId(), "y" } };
	def->outputs = { { DoubleVectorMapNodeData::typeId(), "series" } };
	setNodeDef(def);

	for (int i = 0; i < 2; ++i)
	{
		mNodeDataVariables.push_back(std::make_shared<DoubleVectorNodeData>("", DoubleVector()));
	}
}

std::shared_ptr<QtNodes::NodeData> DataSeriesBuilderNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{	
	int inputIndex = 1;
	for (const DoubleVectorNodeDataPtr& data : mNodeDataVariables)
	{
		updateVariable(*inputs[inputIndex], *data);
		++inputIndex;
	}
	mPrevTime = mContext->timeSource->getTime();

	skybolt::StringVector seriesNames = getDataOrDefault<StringVectorNodeData>(inputs[0].get());
	std::map<std::string, DoubleVectorNodeDataPtr> data;

	int unnamedVariableIndex = 1;
	for (int i = 0; i < mNodeDataVariables.size(); ++i)
	{
		if (i >= seriesNames.size() || seriesNames[i].empty())
		{
			seriesNames.push_back("unnamed" + std::to_string(unnamedVariableIndex++));
		}
		data[seriesNames[i]] = mNodeDataVariables[i];
	}

	auto nodeData = std::make_shared<DoubleVectorMapNodeData>(def->outputs[outputIndex].name, data);

	int i = 0;
	for (const std::string& seriesName : seriesNames)
	{
		const std::string& prevSeriesName = (i < mPrevSeriesNames.size()) ? mPrevSeriesNames[i] : "";
		if (seriesName == prevSeriesName)
		{
			if (!seriesName.empty())
				nodeData->changedKeys.insert(seriesName);
		}
		else
		{
			if (!prevSeriesName.empty())
				nodeData->removedKeys.insert(prevSeriesName);
			if (!seriesName.empty())
				nodeData->addedKeys.insert(seriesName);
		}
		++i;
	}

	mPrevSeriesNames = seriesNames;

	return nodeData;
}

void DataSeriesBuilderNdm::updateVariable(const QtNodes::NodeData& input, DoubleVectorNodeData& output) const
{
	output.addedRange = IntRangeInclusive();
	output.changedRange = IntRangeInclusive();
	output.removedRange = IntRangeInclusive();

	if (mContext->timeSource->getTime() == mPrevTime) // updating another input at the same time. Needed because node graph is push instead of pull. TODO: can remove this if we change to a pull system.
	{
		if (!output.data.empty())
		{
			output.data.back() = getDataOrDefault<DoubleNodeData>(&input);

			int index = (int)output.data.size() - 1;
			output.changedRange = IntRangeInclusive(index, index);
		}
	}
	else if (mContext->timeSource->getTime() > mPrevTime)
	{
		output.data.push_back(getDataOrDefault<DoubleNodeData>(&input));

		int index = (int)output.data.size() - 1;
		output.addedRange = IntRangeInclusive(index, index);
	}
}
