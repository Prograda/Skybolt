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
