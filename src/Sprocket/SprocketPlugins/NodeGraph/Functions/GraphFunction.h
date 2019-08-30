/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "FlowFunction.h"
#include <nodes/FlowScene>
#include <nodes/Node>

class GraphFunction : public FlowFunction
{
public:
	GraphFunction(const NodeDefPtr& nodeDef, const std::shared_ptr<QtNodes::DataModelRegistry>& dataModelRegistry);

	void addPort(QtNodes::PortType portType, QtNodes::PortIndex portIndex, QtNodes::NodeDataType& dataType) override;
	void movePort(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex) override;
	void removePort(QtNodes::PortType portType, QtNodes::PortIndex portIndex) override;

	NodeDataPtrVector eval(const NodeDataPtrVector& inputs) const override;

	QtNodes::FlowScene* getScene() const { return mScene.get(); }

private:
	NodeDefPtr mInputsNodeDef;
	NodeDefPtr mOutputsNodeDef;
	std::unique_ptr<QtNodes::FlowScene> mScene;
};
