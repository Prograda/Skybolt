/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"

class AdditionNdm : public SimpleNdm
{
public:
	AdditionNdm(NodeContext* context)
	{
		NodeDefPtr def = std::make_shared<NodeDef>();
		def->name = Name();
		def->inputs = { { DoubleNodeData::typeId(), "a" },{ DoubleNodeData::typeId(), "b" } };
		def->outputs = { { DoubleNodeData::typeId(), "r" } };
		setNodeDef(def);
	}

	static QString Name() { return "Add"; }

	std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const override
	{
		double v = getDataOrDefault<DoubleNodeData>(inputs[0].get()) + getDataOrDefault<DoubleNodeData>(inputs[1].get());
		return toNodeData<DoubleNodeData>(outputIndex, v);
	}
};
