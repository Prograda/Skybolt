/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropertyModel.h"

#include <boost/scope_exit.hpp>
#include <assert.h>

void PropertiesModel::update()
{
	mCurrentlyUpdating = true;
	BOOST_SCOPE_EXIT(&mCurrentlyUpdating) {
        mCurrentlyUpdating = false;
    } BOOST_SCOPE_EXIT_END

	for (const auto& entry : mPropertyUpdaters)
	{
		entry.second(*entry.first);
	}
}

void PropertiesModel::addProperty(const QtPropertyPtr& property, QtPropertyUpdater updater, QtPropertyApplier applier)
{
	mProperties.push_back(property);
	if (updater)
	{
		mPropertyUpdaters[property] = updater;
	}

	if (applier)
	{
		connect(property.get(), &QtProperty::valueChanged, this, [=]() {
			if (!mCurrentlyUpdating)
			{
				applier(*property);
			}
		});
	}
}

VariantPropertyPtr PropertiesModel::createVariantProperty(const QString& name, const QVariant& value)
{
	auto property = std::make_shared<VariantProperty>();
	property->name = name;
	property->value = value;
	return property;
}

EnumPropertyPtr PropertiesModel::createEnumProperty(const QString& name, const QStringList& values, int value)
{
	auto property = std::make_shared<EnumProperty>();
	property->name = name;
	property->values = values;
	property->value = value;
	return property;
}

TablePropertyPtr PropertiesModel::createTableProperty(const QString& name, const QStringList& fieldNames, const std::vector<TableRecord>& records)
{
	auto property = std::make_shared<TableProperty>();
	property->name = name;
	property->fieldNames = fieldNames;
	property->records = records;
	return property;
}

typedef std::function<void(int index, const TableRecord& record)> RecordAdded;
typedef std::function<void(int oldIndex, int newIndex)> RecordMoved;
typedef std::function<void(int index)> RecordRemoved;
void calcDiff(std::vector<TableRecord> oldRecords, const std::vector<TableRecord>& newRecords, RecordAdded addedFn, RecordMoved movedFn, RecordRemoved removedFn)
{
	auto getIndex = [&](QUuid id)
	{
		int i = 0;
		for (const TableRecord& record : oldRecords)
		{
			if (record.uuid == id)
			{
				return i;
			}
			++i;
		}
		return -1;
	};

	int i = 0;
	for (const TableRecord& record : newRecords)
	{
		int index = getIndex(record.uuid);
		if (index == -1)
		{
			addedFn(i, record);
			oldRecords.insert(oldRecords.begin() + i, record);
		}
		else if (index != i)
		{
			movedFn(index, i);
			oldRecords.erase(oldRecords.begin() + index);
			oldRecords.insert(oldRecords.begin() + i, record);
		}
		++i;
	}

	for (i = (int)newRecords.size(); i < (int)oldRecords.size(); ++i)
	{
		removedFn(i);
	}
}

PropertiesModel::PropertiesModel()
{
}

PropertiesModel::PropertiesModel(const std::vector<QtPropertyPtr>& properties) :
	mProperties(properties)
{
}
