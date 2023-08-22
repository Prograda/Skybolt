/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TableRecord.h"

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