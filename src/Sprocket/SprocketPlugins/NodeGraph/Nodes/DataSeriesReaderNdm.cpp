/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DataSeriesReaderNdm.h"
#include "CsvParser.h"
#include "NodeContext.h"

#include <SkyboltCommon/Range.h>

using skybolt::StringVector;

DoubleVectorMap readDataSeriesFile(const std::string& filename, const StringVector& fieldNames, int headerLineCount)
{
	using namespace aria::csv;
	DoubleVectorMap result;

	std::ifstream f(filename);
	if (f.is_open())
	{
		const int bufferSize = 1024;
		char buffer[bufferSize];
		for (int i = 0; i < headerLineCount; ++i)
		{
			f.getline(buffer, bufferSize);
		}
		
		CsvParser parser(f);

		int column = 0;
		int row = 0;
		while (true)
		{
			auto field = parser.next_field();
			if (field.type == FieldType::DATA)
			{
				if (column < fieldNames.size())
				{
					const std::string& name = fieldNames[column];
					auto it = result.find(name);
					if (it == result.end())
					{
						it = result.insert(DoubleVectorMap::value_type(name, std::make_shared<DoubleVectorNodeData>(QString::fromStdString(name), DoubleVector()))).first;
					}

					try
					{
						double value = std::stod(*field.data);
						DoubleVector& series = it->second->data;
						series.push_back(value);
					}
					catch (const std::invalid_argument&)
					{
						const int maxLength = 8;
						std::string shortField = field.data->substr(0, maxLength);
						if (field.data->length() > maxLength)
						{
							shortField += "...";
						}
						throw std::runtime_error("Field value is not a number: " + shortField);
					}
				}
				++column;
			}
			else if (field.type == FieldType::ROW_END)
			{
				column = 0;
				++row;
			}
			else if (field.type == FieldType::CSV_END)
			{
				break;
			}
		}
	}
	else
	{
		throw std::runtime_error("Could not open file " + filename);
	}
	f.close();

	return result;
}

DataSeriesReaderNdm::DataSeriesReaderNdm(NodeContext* context) :
	mContext(context)
{
	assert(mContext);

	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = DataSeriesReaderNdm::name();
	def->inputs = { { FilenameNodeData::typeId(), "filename" },{ StringVectorNodeData::typeId(), "fields" },{ IntNodeData::typeId(), "headerLines" } };
	def->outputs = { { DoubleVectorMapNodeData::typeId(), "series" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> DataSeriesReaderNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	std::string filename = mContext->fileLocator(getDataOrDefault<FilenameNodeData>(inputs[0].get()), skybolt::file::FileLocatorMode::Required).string();
	DoubleVectorMap data = readDataSeriesFile(
		filename,
		getDataOrDefault<StringVectorNodeData>(inputs[1].get()),
		getDataOrDefault<IntNodeData>(inputs[2].get())
	);

	auto nodeData = std::make_shared<DoubleVectorMapNodeData>(def->outputs[outputIndex].name, data);
	
	// Store set of added keys
	for (const auto& entry : nodeData->data)
	{
		if (prevResult.find(entry.first) == prevResult.end())
		{
			nodeData->addedKeys.insert(entry.first);
		}
	}

	// Store set of removed keys
	for (const auto& entry : prevResult)
	{
		if (nodeData->data.find(entry.first) == nodeData->data.end())
		{
			nodeData->removedKeys.insert(entry.first);
		}
	}
	prevResult = data;
	return nodeData;
}
