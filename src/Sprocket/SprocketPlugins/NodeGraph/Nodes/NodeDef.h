/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nodes/NodeDataModel>
#include <QString>
#include <vector>

struct NodeDef : QObject
{
	Q_OBJECT;
public:
	QString name;
	std::vector<QtNodes::NodeDataType> inputs;
	std::vector<QtNodes::NodeDataType> outputs;

	inline std::vector<QtNodes::NodeDataType>& getPorts(QtNodes::PortType type)
	{
		return (type == QtNodes::PortType::In) ? inputs : outputs;
	}

signals:
	void portAdded(QtNodes::PortType, QtNodes::PortIndex);
	void portMoved(QtNodes::PortType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex);
	void portRemoved(QtNodes::PortType, QtNodes::PortIndex);
};

typedef std::shared_ptr<NodeDef> NodeDefPtr;