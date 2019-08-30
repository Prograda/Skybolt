/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Vector3Ndm.h"

using namespace skybolt;

Vector3Ndm::Vector3Ndm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { DoubleNodeData::typeId(), "x" },{ DoubleNodeData::typeId(), "y" },{ DoubleNodeData::typeId(), "z" } };
	def->outputs = { { Vector3NodeData::typeId(), "V" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> Vector3Ndm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	sim::Vector3 v(
		getDataOrDefault<DoubleNodeData>(inputs[0].get()),
		getDataOrDefault<DoubleNodeData>(inputs[1].get()),
		getDataOrDefault<DoubleNodeData>(inputs[2].get()));
	return toNodeData<Vector3NodeData>(outputIndex, v);
}
