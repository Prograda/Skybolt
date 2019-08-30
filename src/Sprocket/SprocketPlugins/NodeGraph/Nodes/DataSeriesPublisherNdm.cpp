/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DataSeriesPublisherNdm.h"
#include "NodeContext.h"

using namespace skybolt;

DataSeriesPublisherNdm::DataSeriesPublisherNdm(NodeContext* context) :
	mDataSeriesRegistry(context->dataSeriesRegistry)
{
	assert(mDataSeriesRegistry);

	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {
		{ DoubleVectorMapNodeData::typeId(), "data series" },
		{ StringNodeData::typeId(), "name" }
	};
	setNodeDef(def);
}

DataSeriesPublisherNdm::~DataSeriesPublisherNdm()
{
	mDataSeriesRegistry->remove(mDataSeries.get());
}

void DataSeriesPublisherNdm::eval(const NodeDataVector& inputs) const
{
	const DoubleVectorMapNodeData* inputSeriesMap = dynamic_cast<const DoubleVectorMapNodeData*>(inputs[0].get());
	if (!inputSeriesMap)
	{
		return;
	}

	std::string seriesName = getDataOrDefault<StringNodeData>(inputs[1].get());

	if (!mDataSeries)
	{
		// Create new series
		mDataSeries = std::make_shared<NamedDataSeries>();
		mDataSeries->name = seriesName;
		mDataSeries->data = std::make_shared<DataSeries>();

		mDataSeriesRegistry->add(mDataSeries);

		DataSeries& series = *mDataSeries->data;

		for (const auto& item : inputSeriesMap->data)
		{
			auto key = item.first;
			series.data[key] = item.second->data;
			series.keyAdded(key);
		}
	}
	else
	{
		mDataSeries->name = seriesName;
		DataSeries& series = *mDataSeries->data;

		// Update existing series
		// Added keys
		for (const std::string& key : inputSeriesMap->addedKeys)
		{
			auto data = *inputSeriesMap->data.find(key);
			series.data[key] = data.second->data;
			series.keyAdded(key);
		}
		// Changed keys
		for (const std::string& key : inputSeriesMap->changedKeys)
		{
			auto data = inputSeriesMap->data.find(key)->second;
			series.data[key] = data->data;

			if (!data->addedRange.isEmpty())
			{
				series.valuesAdded(data->addedRange);
			}
			if (!data->changedRange.isEmpty())
			{
				series.valuesChanged(data->changedRange);
			}
			if (!data->removedRange.isEmpty())
			{
				series.valuesRemoved(data->removedRange);
			}
		}
		// Removed keys
		for (const std::string& key : inputSeriesMap->removedKeys)
		{
			series.data.erase(key);
			series.keyRemoved(key);
		}
	}
}
