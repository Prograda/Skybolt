/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "NodeFwd.h"
#include "NodeDataT.h"
#include "Nodedef.h"

#include <nodes/NodeDataModel>
#include <memory>

class SimpleNdm : public QtNodes::NodeDataModel
{
	Q_OBJECT
public:
	void setNodeDef(const NodeDefPtr& nodeDef);

	QString name() const override
	{
		return def->name;
	}

	QString	caption() const override
	{
		return def->name;
	}

	QtNodes::NodeValidationState validationState() const override
	{
		return errorMessage.isEmpty() ? QtNodes::NodeValidationState::Valid : QtNodes::NodeValidationState::Error;
	}

	QString validationMessage() const override
	{
		return errorMessage;
	}

	QJsonObject	save() const override;

	void restore(QJsonObject const &p) override;

	unsigned int nPorts(QtNodes::PortType type) const override
	{
		return (type == QtNodes::PortType::In) ? (int)def->inputs.size() : (int)def->outputs.size();
	}

	QtNodes::NodeDataType dataType(QtNodes::PortType type, QtNodes::PortIndex index) const override
	{
		return (type == QtNodes::PortType::In) ? def->inputs[index] : def->outputs[index];
	}

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex index) override
	{
		return outputParams[index];
	}

	//! For nodes without outputs
	void eval();

	void setInData(std::shared_ptr<QtNodes::NodeData> data, int index) override;

	QWidget* embeddedWidget() override { return nullptr; }

	typedef std::vector<std::shared_ptr<QtNodes::NodeData>> NodeDataVector;

	NodeDataVector* getGuiParams() { return &guiParams; }
	NodeDataVector* getInputParams() { return &inputParams; }

public slots:
	void emitPortAdded(QtNodes::PortType, QtNodes::PortIndex);
	void emitPortMoved(QtNodes::PortType portType, QtNodes::PortIndex oldIndex, QtNodes::PortIndex newIndex);
	void emitPortRemoved(QtNodes::PortType, QtNodes::PortIndex);

signals:
	void inputChanged(int index);

protected:
	virtual std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const { return nullptr; }; //!< For nodes with outputs
	virtual void eval(const NodeDataVector& inputs) const {}; //!< For nodes without outputs

	template <class NodeDataType, typename DataType>
	std::shared_ptr<QtNodes::NodeData> toNodeData(int outputIndex, const DataType& value) const
	{
		return std::make_shared<NodeDataType>(def->outputs[outputIndex].name, value);
	}

	template <class NodeDataType, typename DataType>
	std::shared_ptr<QtNodes::NodeData> toNodeData(int outputIndex, const DataType& value, const skybolt::IntRangeInclusive& dirtyRange) const
	{
		auto result = std::make_shared<NodeDataType>(def->outputs[outputIndex].name, value);
		result->dirtyRange = dirtyRange;
		return result;
	}

private:
	NodeDataVector getMergedParams() const;
	void updateParams();

protected:
	NodeDefPtr def;
	NodeDataVector outputParams;

private:
	NodeDataVector inputParams;
	NodeDataVector guiParams;
	QString errorMessage;
};
