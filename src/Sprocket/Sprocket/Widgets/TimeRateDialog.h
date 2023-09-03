/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDialog>

class TimeRateDialog : public QDialog
{
    Q_OBJECT
public:
    TimeRateDialog(double initialRate, QWidget* parent = nullptr);

	double getRate() const { return mRate; }

Q_SIGNALS:
	void rateChanged(double newRate);
	void closed();

private:
    void closeEvent(QCloseEvent* event) override;

private:
	double mRate;
};