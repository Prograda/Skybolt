/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TableEditorWidget.h"
#include "Icon/SprocketIcons.h"
#include <QBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <assert.h>
#include <set>

TableEditorWidget::TableEditorWidget(const QStringList& fieldNames, QWidget* parent) :
	QWidget(parent)
{
	assert(!fieldNames.empty());
	setLayout(new QVBoxLayout);

	QToolBar* toolbar = new QToolBar();

	QToolButton* createButton = new QToolButton();
	createButton->setIcon(getSprocketIcon(SprocketIcon::Add));
	toolbar->addWidget(createButton);

	QToolButton* deleteButton = new QToolButton();
	deleteButton->setIcon(getSprocketIcon(SprocketIcon::Remove));
	deleteButton->setEnabled(false);
	toolbar->addWidget(deleteButton);

	QObject::connect(createButton, &QToolButton::pressed, [=]()
	{
		mTable->insertRow(mTable->rowCount());

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setData(Qt::UserRole, QUuid::createUuid());
		mTable->setItem(mTable->rowCount() - 1, 0, item);

		emit recordAdded();
	});

	QObject::connect(deleteButton, &QToolButton::pressed, [=]()
	{
		std::set<int> rows;
		for (const QTableWidgetSelectionRange& range : mTable->selectedRanges())
		{
			for (int i = range.topRow(); i <= range.bottomRow(); ++i)
			{
				rows.insert(i);
			}
		}

		for (auto it = rows.rbegin(); it != rows.rend(); ++it)
		{
			int row = *it;
			mTable->removeRow(row);
			emit recordAboutToBeRemoved(row);
		}
	});

	layout()->addWidget(toolbar);

	mTable = new QTableWidget();
	mTable->setColumnCount(fieldNames.size());
	mTable->setHorizontalHeaderLabels(fieldNames);
	layout()->addWidget(mTable);

	QObject::connect(mTable, &QTableWidget::itemSelectionChanged, [=]()
	{
		deleteButton->setEnabled(!mTable->selectedRanges().isEmpty());
	});
}

void TableEditorWidget::setRecords(const std::vector<TableRecord>& records)
{
	mTable->setRowCount((int)records.size());
	int r = 0;
	for (const TableRecord& record : records)
	{
		for (int c = 0; c < mTable->columnCount(); ++c)
		{
			QTableWidgetItem* item = mTable->item(r, c);
			if (!item)
			{
				item = new QTableWidgetItem();
				item->setData(Qt::UserRole, record.uuid);
				mTable->setItem(r, c, item);
			}
			if (c >= record.values.size())
			{
				break;
			}
			item->setText(record.values[c].toString());
		}
		++r;
	}
}

std::vector<TableRecord> TableEditorWidget::getRecords()
{
	std::vector<TableRecord> records(mTable->rowCount());
	
	for (int r = 0; r < mTable->rowCount(); ++r)
	{
		TableRecord& record = records[r];
		record.uuid = mTable->item(r, 0)->data(Qt::UserRole).toUuid();
		for (int c = 0; c < mTable->columnCount(); ++c)
		{
			QTableWidgetItem* item = mTable->item(r, c);
			record.values.push_back(item->text());
		}
	}
	return records;
}
