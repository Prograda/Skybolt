/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FlowFunction.h"
#include <SkyboltCommon/Exception.h>

using namespace QtNodes;

FlowFunction::FlowFunction(const NodeDefPtr& nodeDef) :
	mNodeDef(nodeDef)
{
}

void FlowFunction::addPort(QtNodes::PortType portType, QtNodes::PortIndex portIndex, QtNodes::NodeDataType& dataType)
{
	std::vector<QtNodes::NodeDataType>& ports = mNodeDef->getPorts(portType);
	ports.insert(ports.begin() + portIndex, dataType);
	emit mNodeDef->portAdded(portType, portIndex);
}

void FlowFunction::movePort(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex)
{
	std::vector<QtNodes::NodeDataType>& ports = mNodeDef->getPorts(portType);
	QtNodes::NodeDataType dataType = ports[oldIndex];
	ports.erase(ports.begin() + oldIndex);
	ports.insert(ports.begin() + newIndex, dataType);
	emit mNodeDef->portMoved(portType, oldIndex, newIndex);
}

void FlowFunction::removePort(QtNodes::PortType portType, QtNodes::PortIndex portIndex)
{
	std::vector<QtNodes::NodeDataType>& ports = mNodeDef->getPorts(portType);
	ports.erase(ports.begin() + portIndex);
	emit mNodeDef->portRemoved(portType, portIndex);
}
