/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ErrorLogModel.h"
#include <QWidget>

class QTableWidget;

class ErrorLogWidget : public QWidget
{
public:
    ErrorLogWidget(ErrorLogModel* model, QWidget* parent = nullptr);

private:
    void addItemToTable(const ErrorLogModel::Item& item);

private:
    QTableWidget* mTableWidget;
};
