/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"

class RerangeNdm : public SimpleNdm
{
public:
	RerangeNdm(NodeContext* context)
	{
		NodeDefPtr def = std::make_shared<NodeDef>();
		def->name = Name();
		def->inputs = {
			// TODO: set some good defaults
			{ DoubleNodeData::typeId(), "x" },
			{ DoubleNodeData::typeId(), "x1" },
			{ DoubleNodeData::typeId(), "x2" },
			{ DoubleNodeData::typeId(), "y1" },
			{ DoubleNodeData::typeId(), "y2" }
		};
		def->outputs = { { DoubleNodeData::typeId(), "y" } };
		setNodeDef(def);
	}

	static QString Name() { return "Rerange"; }

	std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const override
	{
		double x = getDataOrDefault<DoubleNodeData>(inputs[0].get());
		double x1 = getDataOrDefault<DoubleNodeData>(inputs[1].get());
		double x2 = getDataOrDefault<DoubleNodeData>(inputs[2].get());
		double y1 = getDataOrDefault<DoubleNodeData>(inputs[3].get());
		double y2 = getDataOrDefault<DoubleNodeData>(inputs[4].get());

		double m = (y2 - y1) / (x2 - x1);
		double c = y1 - m * x1;
		double y = m * x + c;

		return toNodeData<DoubleNodeData>(outputIndex, y);
	}
};
