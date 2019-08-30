/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <SkyboltCommon/Range.h>

class DataSeriesBuilderNdm : public SimpleNdm
{
public:
	DataSeriesBuilderNdm(NodeContext* context);

	static QString Name() { return "DataSeriesBuilder"; }

	virtual std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const;

private:
	void updateVariable(const QtNodes::NodeData& input, DoubleVectorNodeData& output) const;

private:
	NodeContext* mContext;
	mutable std::vector<DoubleVectorNodeDataPtr> mNodeDataVariables;
	mutable double mPrevTime;
	mutable skybolt::StringVector mPrevSeriesNames;
};
