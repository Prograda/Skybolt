/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Nodes/NodeDef.h"
#include <nodes/Node>
#include <memory>
#include <set>

typedef std::shared_ptr<QtNodes::NodeData> NodeDataPtr;
typedef std::vector<NodeDataPtr> NodeDataPtrVector;

class FlowFunction
{
public:
	FlowFunction(const NodeDefPtr& nodeDef);

	std::string getName() const { return mNodeDef->name.toStdString(); }

	const std::vector<QtNodes::NodeDataType>& getInputs() const { return mNodeDef->inputs; }
	const std::vector<QtNodes::NodeDataType>& getOutputs() const { return mNodeDef->outputs; }

	virtual void addPort(QtNodes::PortType portType, QtNodes::PortIndex portIndex, QtNodes::NodeDataType& dataType);
	virtual void movePort(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex);
	virtual void removePort(QtNodes::PortType portType, QtNodes::PortIndex portIndex);

	virtual NodeDataPtrVector eval(const NodeDataPtrVector& inputs) const = 0;

	NodeDefPtr getNodeDef() const { return mNodeDef; }

	void registerUser(QtNodes::NodeDataModel* node) { mUserNodes.insert(node); }
	void unregisterUser(QtNodes::NodeDataModel* node) { mUserNodes.erase(node); }

	int getUserCount() const { return (int)mUserNodes.size(); }

private:
	NodeDefPtr mNodeDef;
	std::set< QtNodes::NodeDataModel*> mUserNodes;
};
