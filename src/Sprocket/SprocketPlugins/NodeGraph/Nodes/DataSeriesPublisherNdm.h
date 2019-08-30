/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <Sprocket/DataSeries/DataSeries.h>

class DataSeriesPublisherNdm : public SimpleNdm
{
public:
	DataSeriesPublisherNdm(NodeContext* context);
	~DataSeriesPublisherNdm();

	static QString Name() { return "DataSeriesPublisher"; }

	void eval(const NodeDataVector& inputs) const override;

private:
	void processNodeData(const DoubleVectorMapNodeData& inputSeriesMap, const DoubleVector& variableX) const;

private:
	std::shared_ptr<DataSeriesRegistry> mDataSeriesRegistry;
	mutable std::shared_ptr<NamedDataSeries> mDataSeries;
};
