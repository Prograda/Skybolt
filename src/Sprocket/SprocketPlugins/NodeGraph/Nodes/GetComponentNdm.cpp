/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GetComponentNdm.h"

GetComponentNdm::GetComponentNdm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { DoubleMapNodeData::typeId(), "map" },{ StringNodeData::typeId(), "name" } };
	def->outputs = { { DoubleNodeData::typeId(), "value" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> GetComponentNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	const DoubleMap& doubleMap = getDataOrDefault<DoubleMapNodeData>(inputs[0].get());
	const std::string& name = getDataOrDefault<StringNodeData>(inputs[1].get());

	auto it = doubleMap.find(name);
	double value = (it != doubleMap.end()) ? it->second : 0;
	return toNodeData<DoubleNodeData>(outputIndex, value);
}
