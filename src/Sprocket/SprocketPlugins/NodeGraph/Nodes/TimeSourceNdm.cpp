/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimeSourceNdm.h"
#include "NodeContext.h"
#include <QLabel>

TimeSourceNdm::TimeSourceNdm(NodeContext* nodeContext) :
	mNodeContext(nodeContext)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->outputs = { { DoubleNodeData::typeId(), "value" } };
	setNodeDef(def);

	mConnections.push_back(nodeContext->timeSource->timeChanged.connect([this](double time) {SimpleNdm::eval(); }));
	SimpleNdm::eval();
}

TimeSourceNdm::~TimeSourceNdm()
{
};

std::shared_ptr<QtNodes::NodeData> TimeSourceNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	return toNodeData<DoubleNodeData>(outputIndex, mNodeContext->timeSource->getTime());
}
