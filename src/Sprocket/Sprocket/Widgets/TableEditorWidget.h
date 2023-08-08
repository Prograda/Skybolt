/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TableRecord.h"
#include <QTableWidget>
#include <functional>
#include <vector>

template <class RecordType>
class RecordConverter
{
public:
	typedef std::function<QVariantList(const RecordType&)> RecordToVariantList;
	typedef std::function<RecordType(const QVariantList&)> VariantListToRecord;

	RecordConverter(RecordToVariantList recordToVariantList, VariantListToRecord variantListToRecord) :
		mRecordToVariantList(recordToVariantList),
		mVariantListToRecord(variantListToRecord)
	{
	}

	std::vector<QVariantList> fromVariant(const std::vector<RecordType>& records)
	{
		std::vector<QVariantList> result;
		for (const RecordType& record : records)
		{
			result.push_back(mRecordToVariantList(record));
		}
		return result;
	}

	std::vector<RecordType> fromVariant(const std::vector<QVariantList>& records)
	{
		std::vector<RecordType> result;
		for (const QVariantList& record : records)
		{
			result.push_back(mVariantListToRecord(record));
		}
		return result;
	}

private:
	RecordToVariantList mRecordToVariantList;
	VariantListToRecord mVariantListToRecord;
};

class TableEditorWidget : public QWidget
{
	Q_OBJECT
public:
	TableEditorWidget(const QStringList& fieldNames, QWidget* parent = nullptr);

	void setRecords(const std::vector<TableRecord>& records);
	
	std::vector<TableRecord> getRecords();

signals:
	void recordAdded();
	void recordAboutToBeRemoved(int index);

private:
	QTableWidget* mTable;
};
