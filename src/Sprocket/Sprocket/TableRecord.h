/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QUuid>
#include <QVariantList>
#include <vector>

struct TableRecord
{
	QUuid uuid;
	QVariantList values;
};

using RecordAdded = std::function<void(int index, const TableRecord& record)>;
using RecordMoved = std::function<void(int oldIndex, int newIndex)>;
using RecordRemoved = std::function<void(int index)>;
void calcDiff(std::vector<TableRecord> oldRecords, const std::vector<TableRecord>& newRecords, RecordAdded addedFn, RecordMoved movedFn, RecordRemoved removedFn);
