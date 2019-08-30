/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FlowFunctionPropertiesModel.h"
#include "Functions/FlowFunction.h"
#include <Sprocket/TableRecord.h>
#include <assert.h>

std::vector<TableRecord> toRecords(const std::vector<QtNodes::NodeDataType>& dataTypes)
{
	std::vector<TableRecord> records;
	for (const QtNodes::NodeDataType& type : dataTypes)
	{
		TableRecord record;
		record.uuid = QUuid::createUuid();
		record.values = { type.id, type.name };
		records.push_back(record);
	}
	return records;
}

QtNodes::NodeDataType fromRecord(const TableRecord& record)
{
	return { record.values[0].toString(), record.values[1].toString() };
}

FlowFunctionPropertiesModel::FlowFunctionPropertiesModel(FlowFunction* flowFunction) :
	mFlowFunction(flowFunction)
{
	assert(mFlowFunction);

	mInputsProperty = createTableProperty("inputs", {"dataType", "name"}, toRecords(mFlowFunction->getInputs()));
	mProperties.push_back(mInputsProperty);

	connect(mInputsProperty.get(), &TableProperty::recordAdded, this, [this](int index, const TableRecord& record) {
		mFlowFunction->addPort(QtNodes::PortType::In, index, fromRecord(record));
	});
	connect(mInputsProperty.get(), &TableProperty::recordMoved, this, [this](int oldIndex, int newIndex) {
		mFlowFunction->movePort(QtNodes::PortType::In, oldIndex, newIndex);
	});
	connect(mInputsProperty.get(), &TableProperty::recordRemoved, this, [this](int index) {
		mFlowFunction->removePort(QtNodes::PortType::In, index);
	});

	mOutputsProperty = createTableProperty("outputs", { "dataType", "name" }, toRecords(mFlowFunction->getOutputs()));
	mProperties.push_back(mOutputsProperty);

	connect(mOutputsProperty.get(), &TableProperty::recordAdded, this, [this](int index, const TableRecord& record) {
		mFlowFunction->addPort(QtNodes::PortType::Out, index, fromRecord(record));
	});
	connect(mOutputsProperty.get(), &TableProperty::recordMoved, this, [this](int oldIndex, int newIndex) {
		mFlowFunction->movePort(QtNodes::PortType::Out, oldIndex, newIndex);
	});
	connect(mOutputsProperty.get(), &TableProperty::recordRemoved, this, [this](int index) {
		mFlowFunction->removePort(QtNodes::PortType::Out, index);
	});
}
