/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimeRateDialog.h"

#include <QDialog>
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QToolBar>

TimeRateDialog::TimeRateDialog(double initialRate, QWidget* parent) :
	QDialog(parent, Qt::Popup | Qt::FramelessWindowHint),
	mRate(initialRate)
{
	setFocusPolicy(Qt::ClickFocus);

	QHBoxLayout* layout = new QHBoxLayout(this);

	const int buttonHeight = 25;

	auto toolBar = new QToolBar(this);

	QPushButton* realtimeButton = new QPushButton("1x");
	realtimeButton->setFixedWidth(buttonHeight);
	realtimeButton->setDefault(false);
	realtimeButton->setAutoDefault(false);

	QPushButton* slowDownButton = new QPushButton("-");
	slowDownButton->setFixedWidth(buttonHeight);
	slowDownButton->setDefault(false);
	slowDownButton->setAutoDefault(false);

	QPushButton* speedUpButton = new QPushButton("+");
	speedUpButton->setFixedWidth(buttonHeight);
	speedUpButton->setDefault(false);
	speedUpButton->setAutoDefault(false);

	QDoubleValidator* validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	validator->setBottom(0);

	auto customRateLineEdit = new QLineEdit(this);
	customRateLineEdit->setFocus();
	customRateLineEdit->setText(QString::number(mRate));
	customRateLineEdit->setMaximumWidth(50);
	customRateLineEdit->setValidator(validator);

	toolBar->addWidget(realtimeButton);
	toolBar->addWidget(slowDownButton);
	toolBar->addWidget(speedUpButton);
	toolBar->addWidget(customRateLineEdit);
	toolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	setMaximumWidth(toolBar->width());

	connect(realtimeButton, &QPushButton::pressed, this, [this, customRateLineEdit] {
		mRate = 1;
		customRateLineEdit->setText(QString::number(mRate));
		emit rateChanged(mRate);
	});

	connect(slowDownButton, &QPushButton::pressed, this, [this, customRateLineEdit] {
		mRate /= 2;
		customRateLineEdit->setText(QString::number(mRate));
		emit rateChanged(mRate);
	});

	connect(speedUpButton, &QPushButton::pressed, this, [this, customRateLineEdit] {
		mRate *= 2;
		customRateLineEdit->setText(QString::number(mRate));
		emit rateChanged(mRate);
	});

	connect(customRateLineEdit, &QLineEdit::editingFinished, this, [this, customRateLineEdit] {
		mRate = customRateLineEdit->text().toDouble();
		emit rateChanged(mRate);
	});

	layout->addWidget(toolBar);
}

void TimeRateDialog::closeEvent(QCloseEvent* event)
{
	emit closed();
}
