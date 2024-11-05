/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ErrorLogWidget.h"

#include <QDateTime>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ErrorLogWidget::ErrorLogWidget(ErrorLogModel* model, QWidget* parent) :
    QWidget(parent)
{
    // Create the table widget with 3 columns
    mTableWidget = new QTableWidget(this);
    mTableWidget->setColumnCount(3);
    mTableWidget->setHorizontalHeaderLabels({"Time", "Severity", "Message"});
    mTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    mTableWidget->setSortingEnabled(true);  // Enable sorting by clicking column headers
    mTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make the table non-editable

    // Create the "Clear" button
    QPushButton* clearButton = new QPushButton("Clear", this);
    connect(clearButton, &QPushButton::clicked, model, [model] {
        model->clear();
        });

    // Layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mTableWidget);
    layout->addWidget(clearButton);

    setLayout(layout);

    connect(model, &ErrorLogModel::itemAppended, this, [this] (const ErrorLogModel::Item& item) {
        addItemToTable(item);
        });

    connect(model, &ErrorLogModel::cleared, this, [this] {
        mTableWidget->setRowCount(0);
        });

    // Add initial items
    for (const auto& item : model->getItems())
    {
        addItemToTable(item);
    }
}

static QString toQString(ErrorLogModel::Severity severity)
{
    switch (severity)
    {
    case ErrorLogModel::Severity::Warning:
        return "Warning";
    case ErrorLogModel::Severity::Error:
        return "Error";
    }
    return "";
}

std::unique_ptr<QTableWidgetItem> createItemWithTooltip(const QString& text)
{
    auto item = std::make_unique<QTableWidgetItem>(text);
    item->setToolTip(text);
    return item;
}

void ErrorLogWidget::addItemToTable(const ErrorLogModel::Item& item)
{
    int row = mTableWidget->rowCount();
    mTableWidget->setRowCount(row + 1);

    // Insert time, severity, and message into the new row
    mTableWidget->setItem(row, 0, createItemWithTooltip(item.dateTime.toString("yyyy-MM-dd HH:mm:ss")).release());
    mTableWidget->setItem(row, 1, new QTableWidgetItem(toQString(item.severity)));
    mTableWidget->setItem(row, 2, createItemWithTooltip(item.message).release());
}